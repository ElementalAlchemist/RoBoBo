use robobo::{Socket, SocketConnection};
use rustls::{ClientConfig, ClientSession, Session};
use rustls_native_certs::load_native_certs;
use std::io::{Read, Write};
use std::net::TcpStream;
use std::sync::Arc;
use std::time::Duration;
use webpki::DNSNameRef;

pub struct RustlsSocket {
	config: Arc<ClientConfig>,
}

impl RustlsSocket {
	pub fn new() -> Self {
		let mut config = ClientConfig::new();
		config.root_store = load_native_certs().expect("Failed to load certificate information");
		Self {
			config: Arc::new(config),
		}
	}
}

impl Socket for RustlsSocket {
	fn create_connection(&self) -> Box<dyn SocketConnection> {
		Box::new(RustlsConnection::new(Arc::clone(&self.config)))
	}
}

struct RustlsConnectionData {
	connection: TcpStream,
	session: ClientSession,
}

pub struct RustlsConnection {
	connection_data: Option<RustlsConnectionData>,
	text_buffer: String,
	config: Arc<ClientConfig>,
}

impl RustlsConnection {
	pub fn new(config: Arc<ClientConfig>) -> Self {
		RustlsConnection {
			connection_data: None,
			text_buffer: String::new(),
			config,
		}
	}
}

impl SocketConnection for RustlsConnection {
	fn connect(&mut self, connect_to: &str, read_timeout: Option<Duration>) -> Result<(), String> {
		let connect_dns = match DNSNameRef::try_from_ascii_str(connect_to) {
			Ok(addr) => addr,
			Err(error) => return Err(format!("Failed to complete DNS lookup: {}", error)),
		};

		let stream = match TcpStream::connect(connect_to) {
			Ok(stream) => stream,
			Err(error) => return Err(format!("{}", error)),
		};
		if let Err(error) = stream.set_read_timeout(read_timeout) {
			return Err(format!("{}", error));
		}
		self.connection_data = Some(RustlsConnectionData {
			connection: stream,
			session: ClientSession::new(&self.config, connect_dns),
		});
		Ok(())
	}

	fn read_line(&mut self) -> Result<String, String> {
		let connection_data = match &mut self.connection_data {
			Some(data) => data,
			None => return Err(String::from("The connection hasn't been set up.")),
		};

		if connection_data.session.wants_read() {
			if let Err(error) = connection_data.session.read_tls(&mut connection_data.connection) {
				return Err(format!("Failed to read bytes: {}", error));
			}
			if let Err(error) = connection_data.session.process_new_packets() {
				return Err(format!("Failed to process packets: {}", error));
			}
			connection_data.session.read_to_string(&mut self.text_buffer);
		}

		let end_of_line = match self.text_buffer.find('\n') {
			Some(pos) => pos + 1,
			None => return Ok(String::new()),
		};

		let line: String = self.text_buffer.drain(0..end_of_line).collect();
		let line_no_ending = line.strip_suffix('\n').unwrap();
		let line_no_ending = match line_no_ending.strip_suffix('\r') {
			Some(line) => line,
			None => line_no_ending,
		};

		Ok(String::from(line_no_ending))
	}

	fn write_line(&mut self, line: &str) -> Result<(), String> {
		let connection_data = match &mut self.connection_data {
			Some(data) => data,
			None => return Err(String::from("The connection hasn't been set up.")),
		};

		connection_data.session.write_all(line.as_bytes());
		if connection_data.session.wants_write() {
			if let Err(error) = connection_data.session.write_tls(&mut connection_data.connection) {
				return Err(format!("{}", error));
			}
		}

		Ok(())
	}

	fn close(&mut self) {
		self.connection_data = None;
	}
}

#[no_mangle]
pub fn spawn() -> Box<dyn Socket> {
	Box::new(RustlsSocket::new())
}
