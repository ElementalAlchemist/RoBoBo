mod config;
use std::error::Error;

pub fn run(config_file_name: &str, debug_level: u32) {
	let config_data = match config::read_config(config_file_name) {
		Ok(config) => config,
		Err(error) => {
			match error {
				config::ConfigError::FileError(err) => {
					eprintln!("An error occurred reading the configuration file: {}", err)
				}
				config::ConfigError::ParseError(err) => eprintln!("{}", err),
			}
			return;
		}
	};
}
