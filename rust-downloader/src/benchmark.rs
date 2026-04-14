// Parallel downloader benchmark in Rust.
//
// Uses reqwest + tokio. Concurrency is bounded via buffer_unordered.
// Configuration mirrors the C++ and Python benchmarks:
//   SERVER_URL, MAX_CONCURRENT, SCENARIOS, DOWNLOAD_DIRECTORY.

use futures::stream::{self, StreamExt};
use sha2::{Digest, Sha256};
use std::env;
use std::path::PathBuf;
use std::time::Instant;
use tokio::fs::File;
use tokio::io::AsyncWriteExt;

fn env_or(key: &str, default: &str) -> String {
    env::var(key).unwrap_or_else(|_| default.to_string())
}

fn env_usize(key: &str, default: usize) -> usize {
    env::var(key)
        .ok()
        .and_then(|v| v.parse().ok())
        .unwrap_or(default)
}

fn parse_scenarios(spec: &str) -> Vec<usize> {
    spec.split(',')
        .filter_map(|s| s.trim().parse().ok())
        .collect()
}

fn file_name_for(dir: &str, url: &str) -> PathBuf {
    let mut h = Sha256::new();
    h.update(url.as_bytes());
    let digest = hex::encode(h.finalize());
    PathBuf::from(dir).join(format!("rust.{}.out", &digest[..16]))
}

async fn download_one(
    client: &reqwest::Client,
    url: String,
    dir: String,
) -> Result<PathBuf, reqwest::Error> {
    let path = file_name_for(&dir, &url);
    let mut resp = client.get(&url).send().await?.error_for_status()?;
    let mut file = File::create(&path)
        .await
        .expect("failed to create output file");
    while let Some(chunk) = resp.chunk().await? {
        file.write_all(&chunk).await.expect("write failed");
    }
    Ok(path)
}

async fn download_batch(urls: Vec<String>, dir: &str, max_concurrent: usize) -> (usize, usize) {
    let client = reqwest::Client::builder()
        .user_agent("penelope-bot")
        .connect_timeout(std::time::Duration::from_secs(30))
        .timeout(std::time::Duration::from_secs(300))
        .pool_max_idle_per_host(max_concurrent)
        .build()
        .expect("failed to build client");

    let ok = std::sync::atomic::AtomicUsize::new(0);
    let err = std::sync::atomic::AtomicUsize::new(0);

    stream::iter(urls)
        .map(|u| download_one(&client, u, dir.to_string()))
        .buffer_unordered(max_concurrent)
        .for_each(|res| async {
            match res {
                Ok(_) => {
                    ok.fetch_add(1, std::sync::atomic::Ordering::Relaxed);
                }
                Err(e) => {
                    eprintln!("download failed: {}", e);
                    err.fetch_add(1, std::sync::atomic::Ordering::Relaxed);
                }
            }
        })
        .await;

    (
        ok.load(std::sync::atomic::Ordering::Relaxed),
        err.load(std::sync::atomic::Ordering::Relaxed),
    )
}

#[tokio::main]
async fn main() {
    let server_url = env_or("SERVER_URL", "http://127.0.0.1:8888");
    let dir = env_or("DOWNLOAD_DIRECTORY", "/tmp");
    let max_concurrent = env_usize("MAX_CONCURRENT", 20);
    let scenarios = parse_scenarios(&env_or("SCENARIOS", "10,50,100,200"));

    println!("===========================================================");
    println!("BENCHMARK: Rust downloader (reqwest + tokio)");
    println!("server={}  max_concurrent={}", server_url, max_concurrent);
    println!("===========================================================");
    println!("  URLs    Wall time");
    println!("-----------------------------------------------------------");

    // Warm up
    let warmup: Vec<String> = (0..1).map(|i| format!("{}/page/{}", server_url, i)).collect();
    download_batch(warmup, &dir, max_concurrent).await;

    for n in scenarios {
        let urls: Vec<String> = (0..n).map(|i| format!("{}/page/{}", server_url, i)).collect();
        let t0 = Instant::now();
        let (ok, err) = download_batch(urls, &dir, max_concurrent).await;
        let elapsed = t0.elapsed().as_secs_f64();
        println!(
            "  {} URLs -> {} ok, {} failed, {:.6} s",
            n, ok, err, elapsed
        );
    }

    println!("-----------------------------------------------------------");
}
