#!/usr/bin/env python3
"""Validation plotter — reads output.csv produced by virt_sig_cond and renders:
  1. Time-series: raw vs filtered for a chosen axis
  2. Spectrum before/after (uses numpy FFT only for verification, not the pipeline)

Usage:
  python3 scripts/plot.py output.csv [--axis az] [--cutoff 40]
"""

import argparse
import sys

import numpy as np
import matplotlib.pyplot as plt


def load_csv(path: str):
    import csv
    rows = []
    with open(path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append({k: float(v) for k, v in row.items()})
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("csv", help="output.csv from virt_sig_cond")
    ap.add_argument("--axis", default="accel_z", help="column to plot (default: accel_z)")
    ap.add_argument("--cutoff", type=float, default=40.0, help="filter cutoff Hz for annotation")
    ap.add_argument("--rate", type=float, default=200.0, help="sample rate Hz")
    args = ap.parse_args()

    rows = load_csv(args.csv)
    if not rows:
        print("Empty CSV", file=sys.stderr)
        sys.exit(1)

    raw_col = args.axis + "_raw"
    filt_col = args.axis + "_filtered"
    if raw_col not in rows[0]:
        print(f"Column '{raw_col}' not found. Available: {list(rows[0].keys())}", file=sys.stderr)
        sys.exit(1)

    raw  = np.array([r[raw_col]  for r in rows])
    filt = np.array([r[filt_col] for r in rows])
    t    = np.arange(len(raw)) / args.rate

    fig, (ax_time, ax_spec) = plt.subplots(2, 1, figsize=(12, 8))

    # --- Time series ---
    ax_time.plot(t, raw,  alpha=0.6, label="raw",      linewidth=0.8)
    ax_time.plot(t, filt, alpha=0.9, label="filtered", linewidth=1.0)
    ax_time.set_xlabel("Time [s]")
    ax_time.set_ylabel(args.axis)
    ax_time.set_title("Time-series: raw vs filtered")
    ax_time.legend()
    ax_time.grid(True, alpha=0.3)

    # --- Spectrum ---
    N = len(raw)
    window = np.hanning(N)
    freqs = np.fft.rfftfreq(N, 1.0 / args.rate)

    def to_db(x):
        mag = np.abs(np.fft.rfft(x * window)) / (N / 2)
        return 20 * np.log10(np.maximum(mag, 1e-12))

    ax_spec.plot(freqs, to_db(raw),  alpha=0.7, label="raw",      linewidth=0.8)
    ax_spec.plot(freqs, to_db(filt), alpha=0.9, label="filtered", linewidth=1.0)
    ax_spec.axvline(args.cutoff, color="red", linestyle="--", linewidth=0.8,
                    label=f"cutoff {args.cutoff} Hz")
    ax_spec.set_xlabel("Frequency [Hz]")
    ax_spec.set_ylabel("Magnitude [dB]")
    ax_spec.set_title("Spectrum before/after filtering")
    ax_spec.legend()
    ax_spec.grid(True, alpha=0.3)

    plt.tight_layout()
    out_path = args.csv.replace(".csv", "_plot.png")
    plt.savefig(out_path, dpi=150)
    print(f"Saved: {out_path}")
    plt.show()


if __name__ == "__main__":
    main()
