use egui::{Button, FontId, Label, RichText, Ui};
use std::{
    env,
    error::Error,
    io::stdin,
    sync::{Arc, Mutex},
    thread,
    time::Duration,
};
use tokio::{
    runtime::{Handle, Runtime},
    time,
};

use simple_error::{bail, simple_error};

use timothy_pc::{message::Message, timothy::Timothy};

fn main() -> Result<(), Box<dyn Error>> {
    let options = eframe::NativeOptions::default();
    eframe::run_native(
        "Timothy GUI",
        options,
        Box::new(|_cc| Box::new(TimothyGUI::default())),
    );

    Ok(())
}

struct TimothyGUI {
    runtime: Runtime,
    timothy: Arc<Mutex<Option<Timothy>>>,
    status_text: String,
    data: Arc<Mutex<String>>,
}

impl Default for TimothyGUI {
    fn default() -> Self {
        let runtime = Runtime::new().expect("Unable to create runtime");
        let _ = runtime.enter();

        Self {
            runtime,
            timothy: Arc::new(Mutex::new(None)),
            status_text: "".to_owned(),
            data: Arc::new(Mutex::new(String::new())),
        }
    }
}

impl TimothyGUI {
    fn initialize_bluetooth(&mut self) -> Result<(), Box<dyn Error>> {
        self.timothy = Arc::new(Mutex::new(Some(
            self.runtime
                .block_on(async { Timothy::new_bluetooth().await })?,
        )));
        self.initialize_general()
    }

    fn initialize_usb(&mut self) -> Result<(), Box<dyn Error>> {
        self.timothy = Arc::new(Mutex::new(Some(Timothy::new_usb()?)));
        self.initialize_general()
    }

    fn initialize_general(&mut self) -> Result<(), Box<dyn Error>> {
        let data = self.data.clone();
        let timothy = self.timothy.clone();

        thread::spawn(move || {
            let runtime = Runtime::new().unwrap();
            let _ = runtime.enter();
            loop {
                {
                    let mut data = data.lock().unwrap();
                    let mut timothy = timothy.lock().unwrap();
                    if let Some(timothy) = timothy.as_mut() {
                        runtime.block_on(async move {
                            *data = timothy.get_all().await.unwrap();
                        });
                    }
                }

                thread::sleep(Duration::from_secs(1));
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
                ui.add_space(button_size[0] + ui.spacing().button_padding[0] * 2.0);

                // Row 3: More "Control" buttons
                if ui.add_sized(button_size, Button::new("Stop")).clicked() {
                    requests.push(Message::SetLWSpeed(0));
                    requests.push(Message::SetRWSpeed(0));
                }

                // Send messages
                if (!requests.is_empty()) {
                    let mut timothy = self.timothy.lock().unwrap();
                    if let Some(timothy) = &mut *timothy {
                        self.runtime.block_on(async {
                            timothy.send_messages(&requests).await.unwrap();
                        })
                    }
                }
            });
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
            let data = self.data.lock().unwrap();
            ui.label(RichText::new(format!("{}", data)).font(FontId::proportional(40.0)));
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
