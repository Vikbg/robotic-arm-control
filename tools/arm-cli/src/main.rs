use std::env;
use std::fs::OpenOptions;
use std::io::{Read, Write};
use std::process::Command;
use std::thread;
use std::time::{Duration, Instant};

struct Config {
    baud: u32,
    read_ms: u64,
    port: String,
    command: String,
}

fn main() {
    if let Err(err) = run() {
        eprintln!("error: {err}");
        std::process::exit(1);
    }
}

fn run() -> Result<(), String> {
    let config = parse_args(env::args().skip(1).collect())?;
    configure_port(&config.port, config.baud)?;
    let response = send_command(&config.port, &config.command, config.read_ms)?;

    if !response.is_empty() {
        print!("{response}");
    }

    Ok(())
}

fn parse_args(args: Vec<String>) -> Result<Config, String> {
    if args.is_empty() {
        return Err(usage());
    }

    let mut baud = 115_200;
    let mut read_ms = 350;
    let mut index = 0;

    while index < args.len() {
        match args[index].as_str() {
            "--baud" => {
                index += 1;
                let value = args
                    .get(index)
                    .ok_or_else(usage)?
                    .parse::<u32>()
                    .map_err(|_| String::from("invalid baud value"))?;
                baud = value;
                index += 1;
            }
            "--read-ms" => {
                index += 1;
                let value = args
                    .get(index)
                    .ok_or_else(usage)?
                    .parse::<u64>()
                    .map_err(|_| String::from("invalid read timeout value"))?;
                read_ms = value;
                index += 1;
            }
            _ => break,
        }
    }

    let port = args.get(index).ok_or_else(usage)?.clone();
    let action = args.get(index + 1).ok_or_else(usage)?.as_str();
    let tail = &args[index + 2..];

    let command = match action {
        "raw" => {
            if tail.is_empty() {
                return Err(String::from("raw requires a command string"));
            }
            tail.join(" ")
        }
        "set" => {
            if tail.len() != 2 {
                return Err(String::from("set requires: <joint> <angle>"));
            }
            format!("set {} {}", tail[0], tail[1])
        }
        "step" => {
            if tail.len() != 2 {
                return Err(String::from("step requires: <joint> <delta>"));
            }
            format!("step {} {}", tail[0], tail[1])
        }
        "mode" => {
            if tail.len() != 1 {
                return Err(String::from("mode requires: <s|n|d>"));
            }
            format!("mode {}", tail[0])
        }
        "sel" => {
            if tail.len() != 1 {
                return Err(String::from("sel requires: <joint>"));
            }
            format!("sel {}", tail[0])
        }
        "status" => String::from("p"),
        "pins" => String::from("pins"),
        "selfcheck" => String::from("selfcheck"),
        "play" => String::from("play"),
        "stop" => String::from("stop"),
        "rec" => {
            if tail.len() != 1 {
                return Err(String::from("rec requires: <start|stop|clear|status>"));
            }
            format!("rec {}", tail[0])
        }
        _ => return Err(usage()),
    };

    Ok(Config {
        baud,
        read_ms,
        port,
        command,
    })
}

fn configure_port(port: &str, baud: u32) -> Result<(), String> {
    let status = Command::new("stty")
        .args([
            "-F",
            port,
            &baud.to_string(),
            "cs8",
            "-cstopb",
            "-parenb",
            "-icanon",
            "min",
            "0",
            "time",
            "1",
            "-echo",
        ])
        .status()
        .map_err(|err| format!("failed to execute stty: {err}"))?;

    if !status.success() {
        return Err(format!("stty failed for port {port}"));
    }

    Ok(())
}

fn send_command(port: &str, command: &str, read_ms: u64) -> Result<String, String> {
    let mut port_file = OpenOptions::new()
        .read(true)
        .write(true)
        .open(port)
        .map_err(|err| format!("failed to open port {port}: {err}"))?;

    port_file
        .write_all(command.as_bytes())
        .and_then(|_| port_file.write_all(b"\n"))
        .and_then(|_| port_file.flush())
        .map_err(|err| format!("failed to write command: {err}"))?;

    if read_ms == 0 {
        return Ok(String::new());
    }

    thread::sleep(Duration::from_millis(50));
    let deadline = Instant::now() + Duration::from_millis(read_ms);
    let mut output = Vec::new();
    let mut buffer = [0u8; 512];

    while Instant::now() < deadline {
        match port_file.read(&mut buffer) {
            Ok(0) => thread::sleep(Duration::from_millis(25)),
            Ok(count) => output.extend_from_slice(&buffer[..count]),
            Err(err) => return Err(format!("failed to read response: {err}")),
        }
    }

    Ok(String::from_utf8_lossy(&output).to_string())
}

fn usage() -> String {
    String::from(
        "usage:
  arm-cli [--baud 115200] [--read-ms 350] <port> raw <command...>
  arm-cli [--baud 115200] [--read-ms 350] <port> set <joint> <angle>
  arm-cli [--baud 115200] [--read-ms 350] <port> step <joint> <delta>
  arm-cli [--baud 115200] [--read-ms 350] <port> mode <s|n|d>
  arm-cli [--baud 115200] [--read-ms 350] <port> sel <joint>
  arm-cli [--baud 115200] [--read-ms 350] <port> status
  arm-cli [--baud 115200] [--read-ms 350] <port> pins
  arm-cli [--baud 115200] [--read-ms 350] <port> selfcheck
  arm-cli [--baud 115200] [--read-ms 350] <port> rec <start|stop|clear|status>
  arm-cli [--baud 115200] [--read-ms 350] <port> play
  arm-cli [--baud 115200] [--read-ms 350] <port> stop",
    )
}
