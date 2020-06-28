mod config;
mod irc_data;
mod logger;
mod module_communication;
mod module_module;
mod protocol_module;
mod socket_module;
use libc::daemon;
use signal_hook;
use std::sync::atomic::AtomicBool;
use std::sync::Arc;
use std::sync::Mutex;

pub struct ProgramArgs<'a> {
	pub config_file_name: &'a str,
	pub debug_level: u32,
	pub use_log_with_stdout: bool,
}

pub fn run(args: &ProgramArgs) {
	let mut config_data = match config::read_config(&args.config_file_name) {
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

	let log = Arc::new(Mutex::new(logger::Logger::new(
		config_data.get_log_data(),
		args.debug_level,
		args.use_log_with_stdout,
	)));

	let needs_rehash = Arc::new(AtomicBool::new(false));

	if signal_hook::flag::register(signal_hook::SIGHUP, Arc::clone(&needs_rehash)).is_err() {
		eprintln!("Failed to register the SIGHUP signal; the application will not be able to rehash from a signal");
	}

	if args.debug_level == 0 {
		unsafe {
			let daemon_result = daemon(1, 0);
			if daemon_result < 0 {
				eprintln!("Failed to daemonize; running as a foreground process instead");
			}
		}
	}
}
