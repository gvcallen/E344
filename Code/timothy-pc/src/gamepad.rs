use simple_error::simple_error;
use std::error::Error;

use gilrs::{ev, GamepadId, Gilrs};

pub struct State {
    pub speed: f32,
    pub direction: f32,
}
pub struct Gamepad {
    gilrs: Gilrs,
    id: GamepadId,
}

impl Gamepad {
    pub fn new() -> Result<Self, Box<dyn Error>> {
        let gilrs = Gilrs::new().unwrap();
        let id = gilrs
            .gamepads()
            .nth(0)
            .ok_or(simple_error!("No gamepads connected"))?
            .0;

        Ok(Self { gilrs, id })
    }

    pub fn poll(&mut self) {
        while self.gilrs.next_event().is_some() {}
    }

    pub fn state(&self) -> State {
        let gamepad = self.gilrs.gamepad(self.id);

        let right_trigger = gamepad
            .button_data(ev::Button::RightTrigger2)
            .map_or(0.0, |data| data.value());
        let right_stick_x = gamepad.value(ev::Axis::LeftStickX);

        State {
            speed: right_trigger,
            direction: right_stick_x,
        }
    }
}
