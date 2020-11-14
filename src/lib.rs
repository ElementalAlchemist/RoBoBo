mod config;
mod irc_data;
mod module_communication;
mod module_module;
mod protocol_module;
mod socket_module;
use libc::daemon;
use log::error;
use signal_hook;
use simplelog::{CombinedLogger, Config, LevelFilter, SharedLogger, TermLogger, TerminalMode, WriteLogger};
use std::fs;
use std::sync::atomic::AtomicBool;
use std::sync::Arc;

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

	{
		let log_config = config_data.get_log_data();

		let default_log_file_name = String::from("robobo.log");
		let log_file_name = log_config.get("file").unwrap_or(&default_log_file_name);
		let log_file = match fs::File::open(log_file_name) {
			Ok(file) => Some(file),
			Err(error) => {
				eprintln!("Failed to open log file ({}); will not log to file", error);
				None
			}
		};

		let log_level = match log_config.get("level") {
			Some(level) => parse_log_level_string(level),
			None => LevelFilter::Warn,
		};

		let mut loggers: Vec<Box<dyn SharedLogger>> = Vec::new();

		if args.debug_level == 0 {
			// If we're not daemonizing, then we do the terminal logger
			loggers.push(TermLogger::new(log_level, Config::default(), TerminalMode::Mixed));
		}

		if let Some(file) = log_file {
			loggers.push(WriteLogger::new(log_level, Config::default(), file));
		}

		if let Err(error) = CombinedLogger::init(loggers) {
			eprintln!(
				"The logger failed to initialize ({}); no information will be logged",
				error
			);
		}
	}

	let needs_rehash = Arc::new(AtomicBool::new(false));

	if signal_hook::flag::register(signal_hook::SIGHUP, Arc::clone(&needs_rehash)).is_err() {
		error!("Failed to register the SIGHUP signal; the application will not be able to rehash from a signal");
	}

	if args.debug_level == 0 {
		unsafe {
			let daemon_result = daemon(1, 0);
			if daemon_result < 0 {
				error!("Failed to daemonize; running as a foreground process instead");
			}
		}
	}
}

fn parse_log_level_string(log_level: &str) -> LevelFilter {
	match log_level.to_lowercase().as_str() {
		"off" => LevelFilter::Off,
		"error" => LevelFilter::Error,
		"warn" => LevelFilter::Warn,
		"info" => LevelFilter::Info,
		"debug" => LevelFilter::Debug,
		"trace" => LevelFilter::Trace,
		_ => {
			eprintln!("Configured log level wasn't one of the options; defaulting to \"warn\"");
			LevelFilter::Warn
		}
	}
}
