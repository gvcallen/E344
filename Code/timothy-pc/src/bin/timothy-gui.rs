use btleplug::api::{
    Central, Characteristic, Manager as _, Peripheral as _, ScanFilter, ValueNotification,
    WriteType,
};
use btleplug::platform::{Adapter, Manager, Peripheral};
use egui::{Button, FontId, Label, RichText, Style, Ui, Vec2, Visuals};
use fixed::traits::ToFixed;
use futures::{Stream, StreamExt};
use std::sync::atomic::{AtomicBool, Ordering};
use std::thread::Thread;
use std::{env, error::Error, io::stdin, pin::Pin, sync::Arc, thread, time::Duration};
use timothy_pc::gamepad::Gamepad;
use tokio::sync::Mutex;
use tokio::task;
use tokio::{
    runtime::{Handle, Runtime},
    task::{spawn, spawn_blocking},
    time,
};

use simple_error::{bail, simple_error};

use timothy_pc::{
    bluetooth::{self, BluetoothUART},
    message::Message,
    timothy::Timothy,
};

fn main() -> Result<(), Box<dyn Error>> {
    let mut options = eframe::NativeOptions::default();
    options.initial_window_size = Some(Vec2::new(1920.0, 1000.0));

    eframe::run_native(
        "Timothy GUI",
        options,
        Box::new(|creation_context| {
            let style = Style {
                visuals: Visuals::dark(),
                ..Style::default()
            };
            creation_context.egui_ctx.set_pixels_per_point(3.0);
            creation_context.egui_ctx.set_style(style);
            Box::new(TimothyGUI::default())
        }),
    );

    Ok(())
}

struct TimothyGUI {
    runtime: Runtime,
    timothy: Arc<Mutex<Option<Timothy>>>,
    status_text: String,
    telemetry: Arc<Mutex<String>>,
    gamepad_active: Arc<AtomicBool>,
}

impl Default for TimothyGUI {
    fn default() -> Self {
        let runtime = Runtime::new().expect("Unable to create runtime");
        let _ = runtime.enter();

        let telemetry = Arc::new(Mutex::new(String::new()));
        let timothy = Arc::new(Mutex::new(None::<Timothy>));
        let gamepad_active = Arc::new(AtomicBool::new(false));

        {
            let telemetry = telemetry.clone();
            let timothy = timothy.clone();
            let gamepad_active = gamepad_active.clone();
            runtime.spawn(async move {
                loop {
                    {
                        if !gamepad_active.load(Ordering::Relaxed) {
                            let mut text = None::<String>;

                            {
                                let mut timothy = timothy.lock().await;

                                if let Some(timothy) = timothy.as_mut() {
                                    text = Some(timothy.get_all().await.unwrap());
                                }
                            }

                            if let Some(text) = text {
                                *telemetry.lock().await = text;
                            }
                        }

                        time::sleep(Duration::from_secs(1)).await;
                    }
                }
            });
        }

        Self {
            runtime,
            timothy,
            status_text: "".to_owned(),
            telemetry,
            gamepad_active,
        }
    }
}

impl TimothyGUI {
    fn initialize_bluetooth(&mut self) -> Result<(), Box<dyn Error>> {
        *self.timothy.blocking_lock() = Some(
            self.runtime
                .block_on(async { Timothy::new_bluetooth().await })?,
        );
        Ok(())
    }

    fn initialize_usb(&mut self) -> Result<(), Box<dyn Error>> {
        *self.timothy.blocking_lock() = Some(Timothy::new_usb()?);
        Ok(())
    }

    fn initialize_gamepad(&mut self) -> Result<(), Box<dyn Error>> {
        // *self.gamepad.blocking_lock() = Some(Gamepad::new()?);

        // let gamepad = self.gamepad.clone();
        let handle = self.runtime.handle().clone();
        let timothy = self.timothy.clone();
        self.gamepad_active.store(true, Ordering::Relaxed);

        thread::spawn(move || {
            let mut gamepad = Gamepad::new().unwrap();

            loop {
                gamepad.poll();
                let state = gamepad.state();
                let timothy = timothy.clone();
                handle.spawn(async move {
                    let mut timothy = timothy.lock().await;
                    if let Some(timothy) = timothy.as_mut() {
                        timothy.drive(state.speed, state.direction).await.unwrap();
                    }
                });

                thread::sleep(Duration::from_millis(50));
            }
        });

        Ok(())
    }

    fn gui_panel_command(&mut self, ui: &mut Ui) {
        egui::SidePanel::right("bottom_panel_right").show_inside(ui, |ui| {
            let button_size = [150.0, 20.0];

            // Row 2: "Control" buttons
            let mut requests = Vec::new();

            ui.with_layout(egui::Layout::left_to_right(egui::Align::TOP), |ui| {
                if ui.add_sized(button_size, Button::new("Left")).clicked() {
                    requests.push(Message::SetLWSpeed(0));
                    requests.push(Message::SetRWSpeed(15));
                }
                if ui.add_sized(button_size, Button::new("Go")).clicked() {
                    requests.push(Message::SetLWSpeed(15));
                    requests.push(Message::SetRWSpeed(15));
                }
                if ui.add_sized(button_size, Button::new("Right")).clicked() {
                    requests.push(Message::SetLWSpeed(15));
                    requests.push(Message::SetRWSpeed(0));
                }
            });

            ui.add_space(ui.spacing().button_padding[1] * 2.0);
            ui.with_layout(egui::Layout::left_to_right(egui::Align::TOP), |ui| {
                // ui.add_space(button_size[0] + ui.spacing().button_padding[0] * 2.0);

                if ui
                    .add_sized(button_size, Button::new("Left (soft)"))
                    .clicked()
                {
                    requests.push(Message::SetLWSpeed(8));
                    requests.push(Message::SetRWSpeed(15));
                }

                if ui
                    .add_sized(button_size, Button::new("Go (medium)"))
                    .clicked()
                {
                    requests.push(Message::SetLWSpeed(7));
                    requests.push(Message::SetRWSpeed(7));
                }
                if ui
                    .add_sized(button_size, Button::new("Right (soft)"))
                    .clicked()
                {
                    requests.push(Message::SetLWSpeed(15));
                    requests.push(Message::SetRWSpeed(8));
                }
            });
            ui.add_space(ui.spacing().button_padding[1] * 2.0);
            ui.with_layout(egui::Layout::left_to_right(egui::Align::TOP), |ui| {
                ui.add_space(button_size[0] + ui.spacing().button_padding[0] * 2.0);

                if ui.add_sized(button_size, Button::new("Stop")).clicked() {
                    requests.push(Message::SetLWSpeed(0));
                    requests.push(Message::SetRWSpeed(0));
                }
            });

            // Send messages
            if !requests.is_empty() {
                let timothy = self.timothy.clone();
                self.runtime.spawn(async move {
                    let mut timothy = timothy.lock().await;

                    if let Some(timothy) = timothy.as_mut() {
                        timothy.send_messages(&requests).await.unwrap();
                    }
                });
            }
        });
    }

    fn gui_panel_connect(&mut self, ui: &mut Ui) {
        egui::SidePanel::left("bottom_left_panel").show_inside(ui, |ui| {
            ui.add_space(ui.spacing().button_padding[0]);

            let button_size = [150.0, 20.0];

            if ui
                .add_sized(button_size, Button::new("Connect (Bluetooth)"))
                .clicked()
            {
                if self.initialize_bluetooth().is_ok() {
                    self.status_text = "Connected!".to_owned();
                } else {
                    self.status_text = "Could not connect via bluetooth".to_owned();
                }
            }
            if ui
                .add_sized(button_size, Button::new("Connect (USB)"))
                .clicked()
            {
                if self.initialize_usb().is_ok() {
                    self.status_text = "Connected!".to_owned();
                } else {
                    self.status_text = "Could not connect via USB".to_owned();
                }
            }

            if ui
                .add_sized(button_size, Button::new("Activate gamepad"))
                .clicked()
            {
                if self.initialize_gamepad().is_ok() {
                    self.status_text = "Controller detected!".to_owned();
                }
            }

            ui.label(&self.status_text);
        });
    }

    fn gui_panel_control(&mut self, ctx: &egui::Context) {
        egui::TopBottomPanel::bottom("my_bottom_panel")
            .min_height(100.0)
            .show(ctx, |ui| {
                self.gui_panel_connect(ui);
                self.gui_panel_command(ui);
            });
    }

    fn gui_panel_info(&mut self, ctx: &egui::Context) {
        egui::CentralPanel::default().show(ctx, |ui| {
            let string = { self.telemetry.blocking_lock().clone() };
            ui.label(RichText::new(format!("{}", string)).font(FontId::proportional(30.0)));
        });
    }
}

impl eframe::App for TimothyGUI {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        ctx.request_repaint_after(Duration::from_secs(1));
        self.gui_panel_control(ctx);
        self.gui_panel_info(ctx);
    }
}
