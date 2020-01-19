mod config;
mod logger;
use libc::daemon;

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

	let log = logger::Logger::new(config_data.get_log_data(), debug_level);

	if debug_level == 0 {
		unsafe {
			daemon(1, 0);
		}
	}
}
