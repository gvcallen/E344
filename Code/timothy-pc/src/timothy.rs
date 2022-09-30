use std::{error::Error, thread};

use serialport::SerialPort;
use simple_error::bail;

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
        let serialport = serial::connect_by_product("CP210x UART Bridge")
            .ok_or("Could not connect via Serial")?;

        Ok(Timothy::new(TimothyProtocol::Serial(serialport)))
    }

    pub fn set_serial_receive_callback<f>(&mut self) -> Result<(), Box<dyn Error>> {
        // let builder = thread::Builder::new().name("serial_port".into());

        // let mut serial_port = match self.com_method {
        //     TimothyProtocol::Serial(serial_port) => serial_port,
        //     _ => bail!("Serial connection not yet started."),
        // };

        // builder
        //     .spawn(move || -> () {
        //         let mut serial_buf: Vec<u8> = vec![0; 2048];
        //         loop {
        //             match serial_port.read(serial_buf.as_mut_slice()) {
        //                 Ok(bytes_read) => {
        //                     println!("Received from ESP");
        //                     ()
        //                 }
        //                 Err(_) => (),
        //             }
        //         }
        //     })
        //     .unwrap();

        Ok(())
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

    pub async fn send_message(&mut self, request: Message) -> Result<(), Box<dyn Error>> {
        let mut buffer = [0u8; 2];
        request.serialize(&mut buffer)?;

        self.send(&mut buffer).await
    }

    pub async fn receive_message(&mut self, request: Message) -> Result<Message, Box<dyn Error>> {
        let response_length = request.len_response()?;
        let mut buffer = [0u8; 2];

        let length_received = self
            .receive(&mut buffer[0..(response_length as usize)])
            .await?;

        if length_received == (response_length as usize) {
            Message::deserialize(&mut buffer)
        } else {
            bail!("Response from server timed out");
        }
    }
}
