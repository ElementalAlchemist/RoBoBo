use robobo::Socket;
use std::io::{Read, Write};
use std::net::TcpStream;
use std::time::Duration;

pub struct PlaintextSocket {
	connection: Option<TcpStream>,
	text_buffer: String,
}

impl PlaintextSocket {
	fn new() -> Self {
		Self {
			connection: None,
			text_buffer: String::new(),
		}
	}
}

impl Socket for PlaintextSocket {
	fn connect(&mut self, connect_to: &str, read_timeout: Option<Duration>) -> Result<(), String> {
		let stream = match TcpStream::connect(connect_to) {
			Ok(stream) => stream,
			Err(err) => return Err(format!("{}", err)),
		};
		if let Err(error) = stream.set_read_timeout(read_timeout) {
			return Err(format!("{}", error));
		}
		self.connection = Some(stream);
		Ok(())
	}

	fn read_line(&mut self) -> Result<String, String> {
		let connection = if let Some(conn) = &mut self.connection {
			conn
		} else {
			return Err(String::from("The socket is not connected."));
		};

		if let Err(error) = connection.read_to_string(&mut self.text_buffer) {
			return Err(format!("{}", error));
		}

		let end_of_line = match self.text_buffer.find('\n') {
			Some(pos) => pos + 1,
			None => return Ok(String::new()), // If we hit the read timeout without finishing a line, we still want to guarantee the protocol module can perform its occasional tasks if it has any it wants to do, so we return control for now
		};

		let line: String = self.text_buffer.drain(0..end_of_line).collect();
		let line_no_ending = match line.strip_suffix('\n') {
			Some(line) => line,
			None => &line,
		};
		let line_no_ending = match line_no_ending.strip_suffix('\r') {
			Some(line) => line,
			None => line_no_ending,
		};
		Ok(String::from(line_no_ending))
	}

	fn write_line(&mut self, line: &str) -> Result<(), String> {
		let connection = if let Some(conn) = &mut self.connection {
			conn
		} else {
			return Err(String::from("The socket is not connected."));
		};

		let line = format!("{}\r\n", line);
		if let Err(error) = connection.write_all(line.as_bytes()) {
			return Err(format!("{}", error));
		}
		Ok(())
	}

	fn close(&mut self) {
		self.connection = None;
		self.text_buffer = String::new();
	}
}

#[no_mangle]
pub fn spawn() -> Box<dyn Socket> {
	Box::new(PlaintextSocket::new())
}
