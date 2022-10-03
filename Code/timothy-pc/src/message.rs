use std::{error::Error, fmt::Display};

use simple_error::bail;

// Message IDs
const MESSAGE_GET_ALL_ID: u8 = 0;
const MESSAGE_GET_LEFT_WHEEL_CURRENT_ID: u8 = 1;
const MESSAGE_GET_RIGHT_WHEEL_CURRENT_ID: u8 = 2;
const MESSAGE_GET_LEFT_SENSOR_RANGE_ID: u8 = 3;
const MESSAGE_GET_RIGHT_SENSOR_RANGE_ID: u8 = 4;
const MESSAGE_GET_BATTERY_VOLTAGE_ID: u8 = 5;
const MESSAGE_GET_LEFT_WHEEL_SPEED_ID: u8 = 6;
const MESSAGE_GET_RIGHT_WHEEL_SPEED_ID: u8 = 7;
const MESSAGE_SET_LEFT_WHEEL_SPEED_ID: u8 = 100;
const MESSAGE_SET_RIGHT_WHEEL_SPEED_ID: u8 = 101;

// Request message lengths
const MESSAGE_GET_LEFT_WHEEL_CURRENT_LENGTH_REQUEST: u8 = 1;
const MESSAGE_GET_RIGHT_WHEEL_CURRENT_LENGTH_REQUEST: u8 = 1;
const MESSAGE_GET_LEFT_SENSOR_RANGE_LENGTH_REQUEST: u8 = 1;
const MESSAGE_GET_RIGHT_SENSOR_RANGE_LENGTH_REQUEST: u8 = 1;
const MESSAGE_GET_BATTERY_VOLTAGE_LENGTH_REQUEST: u8 = 1;
const MESSAGE_GET_LEFT_WHEEL_SPEED_LENGTH_REQUEST: u8 = 1;
const MESSAGE_GET_RIGHT_WHEEL_SPEED_LENGTH_REQUEST: u8 = 1;
const MESSAGE_GET_ALL_LENGTH_REQUEST: u8 = 1;
const MESSAGE_SET_LEFT_WHEEL_SPEED_LENGTH_REQUEST: u8 = 2;
const MESSAGE_SET_RIGHT_WHEEL_SPEED_LENGTH_REQUEST: u8 = 2;

// Response message lengths
const MESSAGE_GET_LEFT_WHEEL_CURRENT_LENGTH_RESPONSE: u8 = 5;
const MESSAGE_GET_RIGHT_WHEEL_CURRENT_LENGTH_RESPONSE: u8 = 5;
const MESSAGE_GET_LEFT_SENSOR_RANGE_LENGTH_RESPONSE: u8 = 5;
const MESSAGE_GET_RIGHT_SENSOR_RANGE_LENGTH_RESPONSE: u8 = 5;
const MESSAGE_GET_BATTERY_VOLTAGE_LENGTH_RESPONSE: u8 = 5;
const MESSAGE_GET_LEFT_WHEEL_SPEED_LENGTH_RESPONSE: u8 = 2;
const MESSAGE_GET_RIGHT_WHEEL_SPEED_LENGTH_RESPONSE: u8 = 2;
const MESSAGE_GET_ALL_LENGTH_RESPONSE: u8 = 1                               // "ALL" message response length (MUST be updated each time a response is added above)
    + MESSAGE_GET_LEFT_WHEEL_CURRENT_LENGTH_RESPONSE
    + MESSAGE_GET_RIGHT_WHEEL_CURRENT_LENGTH_RESPONSE
    + MESSAGE_GET_LEFT_SENSOR_RANGE_LENGTH_RESPONSE
    + MESSAGE_GET_RIGHT_SENSOR_RANGE_LENGTH_RESPONSE
    + MESSAGE_GET_BATTERY_VOLTAGE_LENGTH_RESPONSE
    + MESSAGE_GET_LEFT_WHEEL_SPEED_LENGTH_RESPONSE
    + MESSAGE_GET_RIGHT_WHEEL_SPEED_LENGTH_RESPONSE;

#[derive(Debug, Clone)]
pub enum Message {
    GetAll(Option<Vec<Message>>),
    GetLWCurrent(Option<f32>),
    GetRWCurrent(Option<f32>),
    GetLSRange(Option<f32>),
    GetRSRange(Option<f32>),
    GetBatteryVoltage(Option<f32>),
    GetLWSpeed(Option<u8>),
    GetRWSpeed(Option<u8>),
    SetLWSpeed(u8),
    SetRWSpeed(u8),
}

impl Message {
    pub fn len(&self) -> u8 {
        match *self {
            Self::GetAll(None) => MESSAGE_GET_ALL_LENGTH_REQUEST,
            Self::GetLWCurrent(None) => MESSAGE_GET_LEFT_WHEEL_CURRENT_LENGTH_REQUEST,
            Self::GetRWCurrent(None) => MESSAGE_GET_RIGHT_WHEEL_CURRENT_LENGTH_REQUEST,
            Self::GetLSRange(None) => MESSAGE_GET_LEFT_SENSOR_RANGE_LENGTH_REQUEST,
            Self::GetRSRange(None) => MESSAGE_GET_RIGHT_SENSOR_RANGE_LENGTH_REQUEST,
            Self::GetBatteryVoltage(None) => MESSAGE_GET_BATTERY_VOLTAGE_LENGTH_REQUEST,
            Self::GetLWSpeed(None) => MESSAGE_GET_LEFT_WHEEL_SPEED_LENGTH_REQUEST,
            Self::GetRWSpeed(None) => MESSAGE_GET_RIGHT_WHEEL_SPEED_LENGTH_REQUEST,
            Self::SetLWSpeed(_) => MESSAGE_SET_LEFT_WHEEL_SPEED_LENGTH_REQUEST,
            Self::SetRWSpeed(_) => MESSAGE_SET_RIGHT_WHEEL_SPEED_LENGTH_REQUEST,

            Self::GetAll(Some(_)) => MESSAGE_GET_ALL_LENGTH_RESPONSE,
            Self::GetLWCurrent(Some(_)) => MESSAGE_GET_LEFT_WHEEL_CURRENT_LENGTH_RESPONSE,
            Self::GetRWCurrent(Some(_)) => MESSAGE_GET_LEFT_SENSOR_RANGE_LENGTH_RESPONSE,
            Self::GetLSRange(Some(_)) => MESSAGE_GET_LEFT_SENSOR_RANGE_LENGTH_RESPONSE,
            Self::GetRSRange(Some(_)) => MESSAGE_GET_RIGHT_SENSOR_RANGE_LENGTH_RESPONSE,
            Self::GetBatteryVoltage(Some(_)) => MESSAGE_GET_BATTERY_VOLTAGE_LENGTH_RESPONSE,
            Self::GetLWSpeed(Some(_)) => MESSAGE_GET_LEFT_WHEEL_SPEED_LENGTH_RESPONSE,
            Self::GetRWSpeed(Some(_)) => MESSAGE_GET_RIGHT_WHEEL_SPEED_LENGTH_RESPONSE,
        }
    }

    pub fn len_response(&self) -> Result<u8, Box<dyn Error>> {
        match *self {
            Self::GetAll(None) => Ok(MESSAGE_GET_ALL_LENGTH_RESPONSE),
            Self::GetLWCurrent(None) => Ok(MESSAGE_GET_LEFT_WHEEL_CURRENT_LENGTH_RESPONSE),
            Self::GetRWCurrent(None) => Ok(MESSAGE_GET_LEFT_SENSOR_RANGE_LENGTH_RESPONSE),
            Self::GetLWSpeed(None) => Ok(MESSAGE_GET_LEFT_WHEEL_SPEED_LENGTH_RESPONSE),
            Self::GetRWSpeed(None) => Ok(MESSAGE_GET_RIGHT_WHEEL_SPEED_LENGTH_RESPONSE),
            Self::GetLSRange(None) => Ok(MESSAGE_GET_LEFT_SENSOR_RANGE_LENGTH_RESPONSE),
            Self::GetRSRange(None) => Ok(MESSAGE_GET_RIGHT_SENSOR_RANGE_LENGTH_RESPONSE),
            Self::GetBatteryVoltage(None) => Ok(MESSAGE_GET_BATTERY_VOLTAGE_LENGTH_RESPONSE),
            _ => bail!("Message (self) is not a request that has a corresponding response"),
        }
    }

    // Input to parse_get and parse_set must be an array of string slices for each subcommand EXCLUDING get/set itself.
    pub fn parse_get(args: &[&str]) -> Result<Self, Box<dyn Error>> {
        if args.len() < 1 {
            bail!("Not enough arguments")
        }

        match args[0] {
            "all" => Ok(Self::GetAll(None)),
            "lwc" => Ok(Self::GetLWCurrent(None)),
            "rwc" => Ok(Self::GetRWCurrent(None)),
            "lsr" => Ok(Self::GetLSRange(None)),
            "rsr" => Ok(Self::GetRSRange(None)),
            "lws" => Ok(Self::GetLWSpeed(None)),
            "rws" => Ok(Self::GetRWSpeed(None)),
            "bat" => Ok(Self::GetBatteryVoltage(None)),
            _ => bail!("Invalid get command"),
        }
    }

    pub fn parse_set(args: &[&str]) -> Result<Self, Box<dyn Error>> {
        if args.len() < 2 {
            bail!("Not enough arguments")
        }

        match args[0] {
            "lws" => {
                let value: u8 = args[1].parse()?;
                if value > 15 {
                    bail!("Speed command too large");
                }

                Ok(Self::SetLWSpeed(value))
            }
            "rws" => {
                let value: u8 = args[1].parse()?;
                if value > 15 {
                    bail!("Speed command too large");
                }

                Ok(Self::SetRWSpeed(value))
            }
            _ => bail!("Invalid set command"),
        }
    }

    pub fn deserialize(buffer: &[u8]) -> Result<Self, Box<dyn Error>> {
        if buffer.len() < 2 {
            bail!("Incompatible message");
        }

        // We use "big endian" because the byte array contains the MSB at the lowest index
        let number = match buffer[0] {
            MESSAGE_GET_ALL_ID => {
                let mut values = Vec::<Message>::new();
                let mut index = 1;
                let buffer_length = buffer.len() as u8;

                while index < buffer_length {
                    values.push(Self::deserialize(&buffer[index as usize..])?);
                    index = index + values.last().unwrap().len();
                }

                Self::GetAll(Some(values))
            }
            MESSAGE_GET_LEFT_WHEEL_CURRENT_ID => {
                Self::GetLWCurrent(Some(f32::from_le_bytes(buffer[1..5].try_into()?)))
            }
            MESSAGE_GET_RIGHT_WHEEL_CURRENT_ID => {
                Self::GetRWCurrent(Some(f32::from_le_bytes(buffer[1..5].try_into()?)))
            }
            MESSAGE_GET_LEFT_SENSOR_RANGE_ID => {
                Self::GetLSRange(Some(f32::from_le_bytes(buffer[1..5].try_into()?)))
            }
            MESSAGE_GET_RIGHT_SENSOR_RANGE_ID => {
                Self::GetRSRange(Some(f32::from_le_bytes(buffer[1..5].try_into()?)))
            }
            MESSAGE_GET_BATTERY_VOLTAGE_ID => {
                Self::GetBatteryVoltage(Some(f32::from_le_bytes(buffer[1..5].try_into()?)))
            }
            MESSAGE_GET_LEFT_WHEEL_SPEED_ID => Self::GetLWSpeed(Some(buffer[1])),
            MESSAGE_GET_RIGHT_WHEEL_SPEED_ID => Self::GetRWSpeed(Some(buffer[1])),
            _ => bail!("Invalid message ID received from server"),
        };

        Ok(number)
    }

    pub fn serialize(&self, buffer: &mut [u8]) -> Result<u8, Box<dyn Error>> {
        match self {
            Self::GetLWCurrent(None) => {
                buffer[0] = MESSAGE_GET_LEFT_WHEEL_CURRENT_ID;
            }
            Self::GetRWCurrent(None) => {
                buffer[0] = MESSAGE_GET_RIGHT_WHEEL_CURRENT_ID;
            }
            Self::GetLSRange(None) => {
                buffer[0] = MESSAGE_GET_LEFT_SENSOR_RANGE_ID;
            }
            Self::GetRSRange(None) => {
                buffer[0] = MESSAGE_GET_RIGHT_SENSOR_RANGE_ID;
            }
            Self::GetBatteryVoltage(None) => {
                buffer[0] = MESSAGE_GET_BATTERY_VOLTAGE_ID;
            }
            Self::GetLWSpeed(None) => {
                buffer[0] = MESSAGE_GET_LEFT_WHEEL_SPEED_ID;
            }
            Self::GetRWSpeed(None) => {
                buffer[0] = MESSAGE_GET_RIGHT_WHEEL_SPEED_ID;
            }
            Self::SetLWSpeed(speed) => {
                if buffer.len() < 2 {
                    bail!("Message buffer does not contain a speed")
                }
                buffer[0] = MESSAGE_SET_LEFT_WHEEL_SPEED_ID;
                buffer[1] = *speed;
            }
            Self::SetRWSpeed(speed) => {
                if buffer.len() < 2 {
                    bail!("Message buffer does not contain a speed")
                }
                buffer[0] = MESSAGE_SET_RIGHT_WHEEL_SPEED_ID;
                buffer[1] = *speed;
            }
            _ => bail!("Invalid message ID to send"),
        }

        Ok(self.len())
    }
}

impl Display for Message {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match *self {
            Self::GetLWCurrent(current) => write!(
                f,
                "Left wheel current:\t{:3.2} mA",
                current.ok_or(std::fmt::Error)?
            ),
            Self::GetRWCurrent(current) => write!(
                f,
                "Right wheel current:\t{:3.2} mA",
                current.ok_or(std::fmt::Error)?
            ),
            Self::GetLSRange(range) => {
                let range = range.ok_or(std::fmt::Error)?;
                if range > 1.0 {
                    write!(f, "Left sensor range:\t> {} m", 1.0)
                } else {
                    write!(f, "Left sensor range:\t{} m", range)
                }
            }
            Self::GetRSRange(range) => {
                let range = range.ok_or(std::fmt::Error)?;
                if range > 1.0 {
                    write!(f, "Right sensor range:\t> {} m", 1.0)
                } else {
                    write!(f, "Right sensor range:\t{} m", range)
                }
            }
            Self::GetBatteryVoltage(voltage) => {
                write!(
                    f,
                    "Battery voltage:\t{:.2} V",
                    voltage.ok_or(std::fmt::Error)?
                )
            }
            Self::GetLWSpeed(speed) => {
                write!(f, "Left wheel speed:\t{}", speed.ok_or(std::fmt::Error)?)
            }
            Self::GetRWSpeed(speed) => {
                write!(f, "Right wheel speed:\t{}", speed.ok_or(std::fmt::Error)?)
            }
            _ => Err(std::fmt::Error),
        }
    }
}
