use std::{error::Error, time::Duration};

use timothy_pc::{
    bluetooth, serial,
    timothy::{Timothy, TimothyProtocol},
};
use tokio::time;

#[tokio::main]
async fn main() -> Result<(), Box<dyn Error>> {
    // let method = "bt";
    let method = "usb";

    let mut timothy = if method == "bt" {
        let peripheral = bluetooth::connect_by_name("Timothy")
            .await
            .ok_or("Could not connect via Bluetooth")?;

        let uart = bluetooth::BluetoothUART::new(peripheral).await?;
        Timothy::new(TimothyProtocol::Bluetooth(uart))
    } else {
        let serialport = serial::connect_by_product("CP210x UART Bridge")
            .ok_or("Could not connect via Serial")?;
        Timothy::new(TimothyProtocol::Serial(serialport))
    };

    timothy.send("hello \0".as_bytes()).await?;

    let mut buffer = [0u8; 20];

    // time::sleep(Duration::from_secs(2)).await;
    let received = timothy.receive(&mut buffer).await?;
    let string1 = String::from_utf8(buffer.to_vec()).unwrap();

    // time::sleep(Duration::from_secs(2)).await;
    timothy.receive(&mut buffer).await?;
    let string2 = String::from_utf8(buffer.to_vec()).unwrap();

    Ok(())
}
