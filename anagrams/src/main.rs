use std::collections::HashMap;
use itertools::Itertools;

type RResult<T> = core::result::Result<T, Box<dyn core::error::Error>>;

fn main() {
    // create_hom().unwrap();
    // sort_hom().unwrap();
    find_anagrams().unwrap();
}

fn find_anagrams() -> RResult<()> {
    let mut reader = csv::Reader::from_path("data/hom-sorted.csv")?;
    let mut data = Vec::new();

    for record in reader.records() {
        let record = record?;

        data.push((record[0].to_string(), record[1].parse::<u128>()?));
    }

    println!("read in");

    for (word, instances) in &data {
        // booorrinnnnggg
        if word.len() < 10 {
            continue;
        }

        if let Some(idx) = find_anagram(word, &data) {
            println!("{word} ({instances}) is an anagram of {} ({})",
                &data[idx].0, &data[idx].1);
        }
    }

    Ok(())
}

fn find_anagram(word: &str, data: &[(String, u128)]) -> Option<usize> {
    for (pos, entry) in data.into_iter().enumerate().rev() {
        if entry.0.len() != word.len() {
            continue;
        }

        if word == entry.0 { 
            continue;
        }

        if entry.1 < 50_000 {
            continue;
        }

        // attempt to filter misspellings.
        // if entry.1 < 1_000 {
        //     continue;
        // }

        // let mut s1w = word.chars().collect::<Vec<_>>()
        //     .windows(3).map(String::from_iter).collect::<Vec<_>>();
        // let s2w = entry.0.chars().collect::<Vec<_>>()
        //     .windows(3).map(String::from_iter).collect::<Vec<_>>();
        // if s1w.iter().any(|s| s2w.contains(&s)) {
        //     continue;
        // }

        // if entry.0.chars().nth(0) == word.chars().nth(0) {
        //     continue;
        // }

        let s1 = word.chars().sorted().collect::<String>();
        let s2 = entry.0.chars().sorted().collect::<String>();

        if s1 == s2 {
            return Some(pos);
        }
    }

    None
}

fn sort_hom() -> RResult<()> {
    let mut reader = csv::Reader::from_path("data/hom.csv")?;
    let mut writer = csv::Writer::from_path("data/hom-sorted.csv")?;

    let mut store = Vec::new();

    for record in reader.records() {
        let record = record?;

        store.push((record[0].to_string(), record[1].parse::<u128>()?));
    }

    store.sort_by(|a, b| b.1.cmp(&a.1));

    for (word, inst) in store {
        writer.write_record(&[word, inst.to_string()])?;
    }

    Ok(())
}

fn create_hom() -> RResult<()> {
    let csvs = std::fs::read_dir("data/")?
        .map(|r| r.map(|e| e.path()))
        .collect::<Result<Vec<_>, std::io::Error>>()?;

    let mut output = csv::Writer::from_path("./data/hom.csv")?;

    for csv in csvs {
        println!("Reading {csv:?}");

        let mut reader = csv::ReaderBuilder::new()
            .has_headers(false)
            .delimiter(b'\t')
            .from_path(&csv)?;

        let mut data = HashMap::new();

        for record in reader.records() {
            let record = record?;

            let name = record[0]
                .rsplit_once('_')
                .map(|p| p.0)
                .unwrap_or_else(|| &record[0])
                .to_lowercase();
            let instances = record[2].parse::<u128>()?;

            data.entry(name)
                .and_modify(|inst| *inst += instances)
                .or_insert(instances);
        }

        println!("Computed hashmap");

        for (word, inst) in data.into_iter() {
            output.write_record(&[word, inst.to_string()])?;
        }

        println!("Wrote out");
    }

    output.flush()?;

    Ok(())
}