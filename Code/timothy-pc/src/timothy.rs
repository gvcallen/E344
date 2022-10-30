use std::{error::Error, sync::Mutex, time::Duration};

use gilrs::{ev::state::GamepadState, GamepadId, Gilrs, GilrsBuilder};
use serialport::SerialPort;
use simple_error::{bail, simple_error};
use tokio::time::{self, timeout};

use crate::{
    bluetooth::{self, BluetoothUART},
    message::Message,
    serial,
};

pub enum TimothyProtocol {
    Serial(Box<dyn SerialPort>),
    Bluetooth(BluetoothUART),
}

pub struct Timothy {
    com_method: TimothyProtocol,
    // active_gamepad: Some<
}

const RECEIVE_DELAY: u64 = 5;
const SEND_DELAY: u64 = 50;

impl Timothy {
    pub fn new(com_method: TimothyProtocol) -> Self {
        Timothy { com_method }
    }

    pub async fn new_bluetooth() -> Result<Self, Box<dyn Error>> {
        let peripheral = bluetooth::connect_by_name("Timothy").await?;

        let uart = bluetooth::BluetoothUART::new(peripheral).await?;
        Ok(Timothy::new(TimothyProtocol::Bluetooth(uart)))
    }

    pub fn new_usb() -> Result<Self, Box<dyn Error>> {
        let serialport =
            serial::connect_by_product("CP210x").ok_or("Could not connect via Serial")?;

        Ok(Timothy::new(TimothyProtocol::Serial(serialport)))
    }

    pub async fn receive(&mut self, buffer: &mut [u8]) -> Result<usize, std::io::Error> {
        match &mut self.com_method {
            // TimothyProtocol::Serial(serialport) => serialport.read(buffer),
            TimothyProtocol::Bluetooth(bluetooth) => bluetooth.receive(buffer).await,
            _ => Ok(0),
        }
    }

    pub async fn send(&mut self, bytes: &[u8]) -> Result<(), Box<dyn Error>> {
        match &mut self.com_method {
            TimothyProtocol::Serial(serialport) => {
                serialport.write(bytes).map(|_| ())?;
            }
            TimothyProtocol::Bluetooth(bluetooth) => {
                bluetooth.send(bytes).await?;
            }
        }

        Ok(())
    }

    pub async fn test(&mut self) -> Result<String, Box<dyn Error>> {
        let _ = self.send(&mut [0; 5]).await;

        Ok("Works".into())
    }

    pub async fn get_all(&mut self) -> Result<String, Box<dyn Error>> {
        let mut result = String::new();

        result.push_str(
            self.send_receive(&Message::GetLWSpeed(None))
                .await?
                .to_string()
                .as_str(),
        );
        result.push('\n');
        result.push_str(
            self.send_receive(&Message::GetLWCurrent(None))
                .await?
                .to_string()
                .as_str(),
        );
        result.push('\n');
        result.push_str(
            self.send_receive(&Message::GetLSRange(None))
                .await?
                .to_string()
                .as_str(),
        );
        result.push('\n');
        result.push_str(
            self.send_receive(&Message::GetRWSpeed(None))
                .await?
                .to_string()
                .as_str(),
        );
        result.push('\n');
        result.push_str(
            self.send_receive(&Message::GetRWCurrent(None))
                .await?
                .to_string()
                .as_str(),
        );
        result.push('\n');
        result.push_str(
            self.send_receive(&Message::GetRSRange(None))
                .await?
                .to_string()
                .as_str(),
        );
        result.push('\n');
        result.push_str(
            self.send_receive(&Message::GetBatteryVoltage(None))
                .await?
                .to_string()
                .as_str(),
        );

        Ok(result)
    }

    pub async fn send_receive(&mut self, request: &Message) -> Result<Message, Box<dyn Error>> {
        if let Err(_) = request.len_response() {
            bail!("Message does not expect a response. Not sending");
        }

        self.send_message(&request).await.unwrap();

        time::sleep(Duration::from_millis(RECEIVE_DELAY)).await;
        self.receive_message(&request).await
    }

    pub async fn go(&mut self) -> Result<(), Box<dyn Error>> {
        self.send_messages(&[Message::SetRWSpeed(15), Message::SetLWSpeed(15)])
            .await
    }

    pub async fn stop(&mut self) -> Result<(), Box<dyn Error>> {
        self.send_messages(&[Message::SetRWSpeed(0), Message::SetLWSpeed(0)])
            .await
    }

    pub async fn left(&mut self) -> Result<(), Box<dyn Error>> {
        self.send_messages(&[Message::SetRWSpeed(15), Message::SetLWSpeed(0)])
            .await
    }

    pub async fn right(&mut self) -> Result<(), Box<dyn Error>> {
        self.send_messages(&[Message::SetRWSpeed(0), Message::SetLWSpeed(15)])
            .await
    }

    // Speed must be [-1.0, 1.0]
    pub async fn drive(&mut self, speed: f32, direction: f32) -> Result<(), Box<dyn Error>> {
        let lw_speed = {
            if direction < 0.0 {
                speed * 15.0 * (1.0 + direction)
            } else {
                speed * 15.0
            }
        };

        let rw_speed = {
            if direction > 0.0 {
                speed * 15.0 * (1.0 - direction)
            } else {
                speed * 15.0
            }
        };

        self.send_message(&Message::SetRWSpeed(rw_speed as u8))
            .await?;
        self.send_message(&Message::SetLWSpeed(lw_speed as u8))
            .await
    }

    pub async fn send_message(&mut self, request: &Message) -> Result<(), Box<dyn Error>> {
        let mut buffer = [0u8; 20];
        let count = request.serialize(&mut buffer)?;

        self.send(&mut buffer[..count as usize]).await
    }

    pub async fn send_messages(&mut self, requests: &[Message]) -> Result<(), Box<dyn Error>> {
        for request in requests {
            self.send_message(request).await?;
            time::sleep(Duration::from_millis(SEND_DELAY)).await;
        }

        Ok(())
    }

    pub async fn receive_message(&mut self, request: &Message) -> Result<Message, Box<dyn Error>> {
        let response_length = request.len_response()?;
        let mut buffer = [0u8; 20];

        let length_received = self
            .receive(&mut buffer[0..(response_length as usize)])
            .await?;

        if length_received == (response_length as usize) {
            Message::deserialize(&mut buffer)
        } else {
            bail!("Did not receive enough bytes from server");
        }
    }
}
