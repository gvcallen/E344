use std::error::Error;

use serialport::SerialPort;

use crate::{bluetooth::BluetoothUART, serial};

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
}
