use std::{error::Error, time::Duration};

use serialport::SerialPort;
use simple_error::bail;
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
}

impl Timothy {
    pub fn new(com_method: TimothyProtocol) -> Self {
        Timothy { com_method }
    }

    pub async fn new_bluetooth() -> Result<Self, Box<dyn Error>> {
        let peripheral = bluetooth::connect_by_name("Timothy")
            .await
            .ok_or("Could not connect via Bluetooth")?;

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
            TimothyProtocol::Serial(serialport) => serialport.read(buffer),
            TimothyProtocol::Bluetooth(bluetooth) => bluetooth.receive(buffer).await,
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

        time::sleep(Duration::from_millis(10)).await;
        self.receive_message(&request).await
    }

    pub async fn send_message(&mut self, request: &Message) -> Result<(), Box<dyn Error>> {
        let mut buffer = [0u8; 20];
        let count = request.serialize(&mut buffer)?;

        self.send(&mut buffer[..count as usize]).await
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
