use serialport::{available_ports, SerialPort, SerialPortType};

pub fn connect_by_product(product: &str) -> Option<Box<dyn SerialPort>> {
    let ports = available_ports().ok()?;

    let mut port_name = None;

    for port in ports {
        if let SerialPortType::UsbPort(info) = port.port_type {
            if let Some(product_candidate) = info.product {
                println!("{}", product_candidate);
                if product_candidate == product {
                    port_name = Some(port.port_name);
                    break;
                }
            }
        }
    }

    let result = serialport::new(port_name?, 115_200).open();

    match result {
        Ok(serialport) => Some(serialport),
        Err(e) => {
            eprintln!("Error: {}", e);
            None
        }
    }
}
