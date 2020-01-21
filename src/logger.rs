use std::collections::HashMap;
use std::fs;
use std::io::Write;

pub struct Logger {
	log_file: Option<fs::File>,
	log_level: LogLevel,
	both_file_and_stdout: bool,
}

impl Logger {
	fn should_log_error(&self, msg_log_level: LogLevel) -> bool {
		match msg_log_level {
			LogLevel::Error => true,
			LogLevel::Warn => match self.log_level {
				LogLevel::Error => false,
				_ => true,
			},
			LogLevel::Info => match self.log_level {
				LogLevel::Debug | LogLevel::Info => true,
				_ => false,
			},
			LogLevel::Debug => match self.log_level {
				LogLevel::Debug => true,
				_ => false,
			},
		}
	}
}

pub enum LogLevel {
	Error,
	Warn,
	Info,
	Debug,
}

impl Logger {
	pub fn new(config: &HashMap<String, String>, debug_level: u32, both_file_and_stdout: bool) -> Logger {
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

		let mut log_level: LogLevel = LogLevel::Warn;
		if config.contains_key("level") {
			let configured_log_level = config["level"].to_lowercase();
			log_level = if configured_log_level == "error" {
				LogLevel::Error
			} else if configured_log_level == "warn" {
				LogLevel::Warn
			} else if configured_log_level == "info" {
				LogLevel::Info
			} else if configured_log_level == "debug" {
				LogLevel::Debug
			} else {
				eprintln!(
					"Failed to parse log level \"{}\"; using log level warn",
					config["level"]
				);
				LogLevel::Warn
			}
		}

		if debug_level >= 3 {
			log_level = LogLevel::Debug;
		} else if debug_level == 2 {
			log_level = match log_level {
				LogLevel::Debug => LogLevel::Info,
				_ => log_level,
			};
		} else if debug_level == 1 {
			log_level = match log_level {
				LogLevel::Debug | LogLevel::Info => LogLevel::Warn,
				_ => log_level,
			};
		}
		// We don't handle debug level 0 here because it means the user didn't choose to specify anything
		// This means we'll daemonize and should use the default/configured log level

		Logger {
			log_file,
			log_level,
			both_file_and_stdout,
		}
	}

	pub fn log(&mut self, log_msg_level: LogLevel, message: &str) {
		if !self.should_log_error(log_msg_level) {
			return;
		}

		if let Some(ref mut file) = self.log_file {
			if let Err(_) = file.write(message.as_bytes()) {
				log_stdout(message);
				self.log_file = None;
			} else if self.both_file_and_stdout {
				log_stdout(message);
			}
		} else {
			log_stdout(message);
		}
	}
}

fn log_stdout(message: &str) {
	println!("{}", message);
}
