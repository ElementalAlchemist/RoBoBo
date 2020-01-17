use std::fs;
use std::io::Write;

pub struct Logger {
	log_file: Option<fs::File>,
	log_level: u32,
}

impl Logger {
	pub fn new(log_file: Option<fs::File>, log_level: u32) -> Logger {
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
