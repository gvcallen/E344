use btleplug::api::{
    Central, Characteristic, Manager as _, Peripheral as _, ScanFilter, ValueNotification,
    WriteType,
};
use btleplug::platform::{Adapter, Manager, Peripheral};
use futures::{Stream, StreamExt};
use std::collections::BTreeSet;
use std::error::Error;
use std::pin::Pin;
use std::thread;
use std::time::Duration;
use tokio::time;
use uuid::{uuid, Uuid};

const BLUETOOTH_SERVICE_UUID: Uuid = uuid!("980e293c-3e62-11ed-b878-0242ac120002");
const BLUETOOTH_CHARACTERISTIC_TX_UUID: Uuid = uuid!("980e2c70-3e62-11ed-b878-0242ac120002");
const BLUETOOTH_CHARACTERISTIC_RX_UUID: Uuid = uuid!("980e2d7e-3e62-11ed-b878-0242ac120002");

// BluetoothUART //
pub struct BluetoothUART {
    peripheral: Peripheral,
    characteristics: BTreeSet<Characteristic>,
    rx_char: usize,
    tx_char: usize,
    tx_stream: Pin<Box<dyn Stream<Item = ValueNotification> + Send>>,
}

impl BluetoothUART {
    pub async fn new(peripheral: Peripheral) -> Result<BluetoothUART, Box<dyn Error>> {
        peripheral.discover_services().await.unwrap();

        let characteristics = peripheral.characteristics();
        let rx_char = characteristics
            .iter()
            .position(|c| c.uuid == BLUETOOTH_CHARACTERISTIC_RX_UUID)
            .ok_or("Rx characteristic not found")?;

        let tx_char = characteristics
            .iter()
            .position(|c| c.uuid == BLUETOOTH_CHARACTERISTIC_TX_UUID)
            .ok_or("Tx characteristic not found")?;

        peripheral
            .subscribe(characteristics.iter().nth(tx_char).unwrap())
            .await
            .unwrap();
        let tx_stream = peripheral.notifications().await.unwrap();

        Ok(BluetoothUART {
            peripheral,
            characteristics,
            rx_char,
            tx_char,
            tx_stream,
        })
    }

    pub async fn send(&self, bytes: &[u8]) -> Result<(), btleplug::Error> {
        let rx_char = self.characteristics.iter().nth(self.rx_char).unwrap();

        self.peripheral
            .write(rx_char, &bytes, WriteType::WithoutResponse)
            .await
    }

    pub async fn receive(&mut self, bytes: &mut [u8]) -> Result<usize, std::io::Error> {
        let stream = self.tx_stream.next().await.ok_or(std::io::Error::new(
            std::io::ErrorKind::TimedOut,
            "Bluetooth notification timeout occurred.",
        ))?;
        let data = &stream.value;

        // Only write up until the length of the buffer
        let length = bytes.len().min(data.len());
        bytes[..length].copy_from_slice(&data[..length]);

        // Return the number of bytes received from the stream
        Ok(data.len())
    }
}

// Generic functions

pub async fn connect_by_name(name: &str) -> Option<Peripheral> {
    let manager = Manager::new().await.unwrap();

    // get the first bluetooth adapter
    let adapters = manager
        .adapters()
        .await
        .expect("No bluetooth adapter available.");
    let central = adapters.into_iter().nth(0).unwrap();

    // start scanning for devices
    central.start_scan(ScanFilter::default()).await.unwrap();
    time::sleep(Duration::from_secs(2)).await;

    // find the device we're interested in
    let peripheral = find_by_name(&central, name).await?;

    let result = peripheral.connect().await;

    match result {
        Ok(_) => Some(peripheral),
        Err(e) => {
            println!("{}", e.to_string());
            None
        }
    }
}

async fn find_by_name(central: &Adapter, peripheral_name: &str) -> Option<Peripheral> {
    let peripherals = central.peripherals().await.unwrap();
    for p in peripherals {
        let properties = p.properties().await.unwrap().unwrap();
        if properties
            .local_name
            .iter()
            .any(|name| name.contains(peripheral_name))
        {
            return Some(p);
        }
    }
    None
}
