use std::{env, error::Error, io::stdin, thread, time::Duration};
use tokio::time;

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

async fn run(timothy: &mut Timothy) -> Result<(), Box<dyn Error>> {
    let mut buffer = String::new();

    // Clear the buffer and read in all words into an array
    stdin().read_line(&mut buffer)?;
    let args: Vec<_> = buffer.split_ascii_whitespace().collect();

    if args.len() < 1 {
        return Ok(());
    }

    // Parse any requests
    match args[0] {
        "go" => timothy.go().await,
        "stop" => timothy.stop().await,
        "left" => timothy.left().await,
        "right" => timothy.right().await,
        "help" => {
            print_help_interactive();
            Ok(())
        }
        "exit" => Ok(()),
        "set" => {
            if let Ok(request) = Message::parse_set(&args[1..]) {
                timothy.send_message(&request).await
            } else {
                bail!("Invalid command");
            }
        }
        "get" => {
            if let Ok(request) = Message::parse_get(&args[1..]) {
                if let Message::GetAll(_) = request {
                    loop {
                        println!("{}", timothy.get_all().await.unwrap());
                        println!("\n");
                    }
                } else {
                    println!(
                        "{}",
                        timothy.send_receive(&request).await.unwrap().to_string()
                    );
                }

                Ok(())
            } else {
                bail!("Invalid command");
            }
        }
        _ => bail!("Invalid command"),
    }
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn Error>> {
    // Collect environment variables and display help if necessary
    let args: Vec<_> = env::args().collect();
    if args.len() == 1 || (args.len() == 2 && args[1] == "help") {
        print_help_start();
        return Ok(());
    }

    if args.len() < 3 {
        bail!("Not enough arguments");
    }

    // Parse any "start" command e.g. to connect to Timothy
    let mut timothy = {
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

    print_help_interactive();
    loop {
        let result = run(&mut timothy).await;

        if let Err(e) = result {
            println!("{}", e);
        }
    }
}
