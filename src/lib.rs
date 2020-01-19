mod config;
mod logger;
use libc::daemon;

pub struct ProgramArgs<'a> {
	pub config_file_name: &'a str,
	pub debug_level: u32,
	pub use_log_with_stdout: bool,
}

pub fn run(args: &ProgramArgs) {
	let config_data = match config::read_config(&args.config_file_name) {
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

	let log = logger::Logger::new(config_data.get_log_data(), args.debug_level);

	if args.debug_level == 0 {
		unsafe {
			daemon(1, 0);
		}
	}
}
