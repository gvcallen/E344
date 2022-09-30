use std::{
    env,
    error::Error,
    io::{stdin, stdout, Read, Write},
};

use simple_error::{bail, simple_error};

use timothy_pc::{message::Message, timothy::Timothy};

const HELP_START: &str = "*** Help ***
    Usage: timc <command> <argument1, argument2, ..>

    Available commands:
    start <subcommand>
         bt             start connection in Bluetooth mode using BLE
         usb            start connection in USB mode via Serial

    ";

const HELP_INTERACTIVE: &str = "*** Interactive Menu ***
    Usage: <command> <argument1, argument2, ..> 

    Example: set lws 8
    
    Available commands: 
        help            display this help
    
        set <subcommand> <value>
            lws         set the left wheel speed [0-15]
            rws         set the right wheel speed [0-15]
        
        go              set left and right wheel speed to full
        left            set right wheel speed to full and left wheel speed to zero
        right           set left wheel speed to full and right wheel speed to zero
        stop            set left and right wheel speed to zero
    
        get <subcommand>
            all         get all information
            lwc         get the left wheel current
            rwc         get the right wheel current
            lsr         get the left sensor range
            rsr         get the right sensor range
            bat         get battery voltage
        
        exit
            exit the program

Enter a command:";

fn print_help_start() {
    println!("{}", HELP_START);
}

fn print_help_interactive() {
    println!("{}", HELP_INTERACTIVE);
}

async fn run(mut timothy: Timothy) -> Result<(), Box<dyn Error>> {
    print_help_interactive();

    let mut buffer = String::new();
    loop {
        buffer.clear();

        stdin().read_line(&mut buffer)?;
        let args: Vec<_> = buffer.split_ascii_whitespace().collect();

        if args.len() < 1 {
            continue;
        }

        let combination_message = match args[0] {
            "go" => Some(vec![Message::SetLWSpeed(15), Message::SetRWSpeed(15)]),
            "stop" => Some(vec![Message::SetLWSpeed(0), Message::SetRWSpeed(0)]),
            "left" => Some(vec![Message::SetLWSpeed(0), Message::SetRWSpeed(15)]),
            "right" => Some(vec![Message::SetLWSpeed(15), Message::SetRWSpeed(0)]),
            _ => None,
        };

        if let Some(messages) = combination_message {
            for m in messages {
                timothy.send_message(m).await.unwrap();
            }
            continue;
        }

        let message = match args[0] {
            "help" => {
                print_help_interactive();
                continue;
            }
            "exit" => return Ok(()),
            "set" => Message::parse_set(&args[1..]),
            "get" => Message::parse_get(&args[1..]),
            _ => Err(simple_error!("Invalid command").into()),
        };

        match message {
            Ok(message) => timothy.send_message(message).await.unwrap(),
            Err(e) => println!("{}", e.to_string()),
        }
    }
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn Error>> {
    let args: Vec<_> = env::args().collect();

    if args.len() == 1 || (args.len() == 2 && args[1] == "help") {
        print_help_start();
        return Ok(());
    }

    if args.len() < 3 {
        bail!("Not enough arguments");
    }

    let timothy = {
        if args[1] == "start" {
            match args[2].as_str() {
                "bt" => {
                    println!("Connecting via bluetooth...");
                    Timothy::new_bluetooth().await?
                }
                "usb" => {
                    print!("Connecting via USB...");
                    Timothy::new_usb()?
                }
                _ => bail!("Invalid connection type"),
            }
        } else {
            bail!("First parameter must be 'start'");
        }
    };

    println!("Connected!");

    run(timothy).await
}
