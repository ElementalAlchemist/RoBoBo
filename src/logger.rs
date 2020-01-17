use std::collections::HashMap;
use std::fs;
use std::io::Write;
use std::num::ParseIntError;

pub struct Logger {
	log_file: Option<fs::File>,
	log_level: u32,
}

impl Logger {
	pub fn new(config: &HashMap<String, String>, debug_level: u32) -> Logger {
		let mut log_file: Option<fs::File> = None;
		if debug_level == 0 {
			let mut log_file_name = "robobo.log";
			if config.contains_key("file") {
				log_file_name = &config["file"];
			}
			let log_open_result = fs::File::open(log_file_name);
			if let Ok(file) = log_open_result {
				log_file = Some(file);
			} else {
				eprintln!("Failed to open log file for writing; no log will be created");
			}
		}

		let mut log_level: u32 = 0;
		if config.contains_key("level") {
			log_level = config["level"].parse().unwrap_or_else(|error: ParseIntError| {
				eprintln!("Failed to read integer log level ({}); using log level 0", error);
				0
			});
		}

		Logger { log_file, log_level }
	}

	pub fn log(&mut self, log_msg_level: u32, message: &str) {
		if log_msg_level < self.log_level {
			return;
		}

		if let Some(ref mut file) = self.log_file {
			if let Err(_) = file.write(message.as_bytes()) {
				log_stdout(message);
				self.log_file = None;
			}
		} else {
			log_stdout(message);
		}
	}
}

fn log_stdout(message: &str) {
	println!("{}", message);
}
