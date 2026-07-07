from __future__ import annotations

import argparse
from pathlib import Path

import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
import pandas as pd


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Load benchmark_results.csv and generate separate PNG reports."
    )
    parser.add_argument(
        "--input",
        default="benchmark_results.csv",
        help="CSV input file produced by the benchmark run.",
    )
    parser.add_argument(
        "--output-dir",
        default="output",
        help="Directory where the PNG reports will be written.",
    )
    return parser.parse_args()


def load_data(csv_path: Path) -> pd.DataFrame:
    df = pd.read_csv(csv_path, sep=";")
    expected_columns = {
        "run_id",
        "structure",
        "collision",
        "key_type",
        "n_tweets",
        "time_ms",
        "size_bytes",
    }
    missing = expected_columns.difference(df.columns)
    if missing:
        raise ValueError(f"Missing required columns: {sorted(missing)}")

    df = df.copy()
    df["n_tweets"] = pd.to_numeric(df["n_tweets"], errors="raise")
    df["time_ms"] = pd.to_numeric(df["time_ms"], errors="raise")
    df["size_bytes"] = pd.to_numeric(df["size_bytes"], errors="raise")
    return df


def print_statistics(df: pd.DataFrame) -> None:
    summary = (
        df.groupby(["structure", "collision", "key_type", "n_tweets"], as_index=False)
        .agg(
            mean_time_ms=("time_ms", "mean"),
            std_time_ms=("time_ms", "std"),
            mean_size_bytes=("size_bytes", "mean"),
            runs=("time_ms", "count"),
        )
        .sort_values(["n_tweets", "structure", "collision", "key_type"])
    )

    print("\nResumen por estructura, colisión, tipo de clave y checkpoint:")
    print(summary.to_string(index=False, float_format=lambda value: f"{value:.4f}"))


def write_text_report(df: pd.DataFrame, output_path: Path) -> None:
    grouped = (
        df.groupby(["structure", "collision", "key_type", "n_tweets"], as_index=False)
        .agg(
            mean_time_ms=("time_ms", "mean"),
            std_time_ms=("time_ms", "std"),
            mean_size_bytes=("size_bytes", "mean"),
            runs=("time_ms", "count"),
        )
        .sort_values(["n_tweets", "structure", "collision", "key_type"])
    )

    overall = (
        df.groupby(["structure", "collision", "key_type"], as_index=False)
        .agg(
            mean_time_ms=("time_ms", "mean"),
            std_time_ms=("time_ms", "std"),
            mean_size_bytes=("size_bytes", "mean"),
            runs=("time_ms", "count"),
        )
        .sort_values(["structure", "collision", "key_type"])
    )

    final_checkpoint = int(grouped["n_tweets"].max())
    final_memory = (
        grouped[grouped["n_tweets"] == final_checkpoint]
        .loc[:, ["structure", "collision", "key_type", "mean_size_bytes"]]
        .copy()
        .sort_values(["structure", "collision", "key_type"])
    )

    checkpoints = sorted(df["n_tweets"].unique())
    configs = grouped[["structure", "collision", "key_type"]].drop_duplicates()

    def fmt_bytes(value_bytes: float) -> str:
        if value_bytes >= 1024.0 * 1024.0:
            return f"{value_bytes / (1024.0 * 1024.0):.2f} MB"
        if value_bytes >= 1024.0:
            return f"{value_bytes / 1024.0:.2f} KB"
        return f"{value_bytes:.0f} B"

    lines = []
    lines.append("REPORTE DE BENCHMARK")
    lines.append("")
    lines.append(f"Archivo origen: benchmark_results.csv")
    lines.append(f"Total de corridas registradas: {len(df)}")
    lines.append(f"Configuraciones evaluadas: {len(configs)}")
    lines.append(f"Checkpoints evaluados: {len(checkpoints)} ({checkpoints[0]}..{checkpoints[-1]})")
    lines.append("Cada grupo del reporte corresponde a 20 repeticiones.")
    lines.append("")
    lines.append("RESUMEN POR GRUPOS DE 20 EXPERIMENTOS")
    lines.append("structure;collision;key_type;n_tweets;runs;mean_time_ms;std_time_ms;mean_size_bytes;size_human")
    for _, row in grouped.iterrows():
        lines.append(
            f"{row['structure']};{row['collision']};{row['key_type']};{int(row['n_tweets'])};{int(row['runs'])};"
            f"{row['mean_time_ms']:.4f};{row['std_time_ms']:.4f};"
            f"{int(round(row['mean_size_bytes']))};{fmt_bytes(row['mean_size_bytes'])}"
        )
    lines.append("")
    lines.append("RESUMEN GLOBAL POR CONFIGURACION")
    lines.append("structure;collision;key_type;runs;mean_time_ms;std_time_ms;mean_size_bytes;size_human")
    for _, row in overall.iterrows():
        lines.append(
            f"{row['structure']};{row['collision']};{row['key_type']};{int(row['runs'])};"
            f"{row['mean_time_ms']:.4f};{row['std_time_ms']:.4f};"
            f"{int(round(row['mean_size_bytes']))};{fmt_bytes(row['mean_size_bytes'])}"
        )
    lines.append("")
    lines.append(f"MEMORIA PROMEDIO EN EL CHECKPOINT FINAL ({final_checkpoint})")
    lines.append("structure;collision;key_type;mean_size_bytes;size_human")
    for _, row in final_memory.iterrows():
        lines.append(
            f"{row['structure']};{row['collision']};{row['key_type']};"
            f"{int(round(row['mean_size_bytes']))};"
            f"{fmt_bytes(row['mean_size_bytes'])}"
        )

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def human_size(value_bytes: float, unit: str) -> float:
    if unit == "B":
        return value_bytes
    if unit == "KB":
        return value_bytes / 1024.0
    if unit == "MB":
        return value_bytes / (1024.0 * 1024.0)
    raise ValueError(f"Unsupported unit: {unit}")


def pick_size_unit(max_bytes: float) -> str:
    if max_bytes >= 1024.0 * 1024.0:
        return "MB"
    if max_bytes >= 1024.0:
        return "KB"
    return "B"


def plot_comparacion_memoria(df: pd.DataFrame, output_path: Path) -> None:
    checkpoint = int(df["n_tweets"].max())
    checkpoint_df = df[df["n_tweets"] == checkpoint].copy()

    memory = (
        checkpoint_df.groupby(["structure", "collision", "key_type"], as_index=False)
        .agg(size_bytes=("size_bytes", "mean"))
        .sort_values(["structure", "collision", "key_type"])
    )

    unit = pick_size_unit(float(memory["size_bytes"].max()))
    memory["size_display"] = memory["size_bytes"].map(lambda value: human_size(value, unit))
    memory["label"] = memory["key_type"].map({"int": "user_id", "string": "screen_name"})
    memory["series"] = memory["structure"] + " / " + memory["collision"]

    series_order = list(memory["series"].drop_duplicates())
    label_order = ["user_id", "screen_name"]

    fig, ax = plt.subplots(figsize=(12, 6), constrained_layout=True)
    bar_width = 0.35
    x_positions = list(range(len(series_order)))

    for offset, label in enumerate(label_order):
        values = []
        for series in series_order:
            match = memory[(memory["series"] == series) & (memory["label"] == label)]
            values.append(float(match["size_display"].iloc[0]) if not match.empty else 0.0)
        positions = [x + (offset - 0.5) * bar_width for x in x_positions]
        bars = ax.bar(positions, values, width=bar_width, label=label)
        for bar, value in zip(bars, values):
            ax.text(
                bar.get_x() + bar.get_width() / 2,
                bar.get_height(),
                f"{value:.2f}",
                ha="center",
                va="bottom",
                fontsize=8,
            )

    ax.set_xticks(x_positions)
    ax.set_xticklabels(series_order, rotation=25, ha="right")
    ax.set_ylabel(f"Tamaño en memoria ({unit})")
    ax.set_title(f"Comparación del tamaño de las estructuras en el checkpoint final ({checkpoint})")
    ax.legend(title="Clave")
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=200, bbox_inches="tight")
    plt.close(fig)


def plot_escalabilidad_creacion(df: pd.DataFrame, output_path: Path) -> None:
    grouped = (
        df.groupby(["structure", "collision", "key_type", "n_tweets"], as_index=False)
        .agg(
            mean_time_ms=("time_ms", "mean"),
            std_time_ms=("time_ms", "std"),
            runs=("time_ms", "count"),
        )
        .sort_values(["structure", "collision", "key_type", "n_tweets"])
    )

    structures = list(grouped["structure"].drop_duplicates())
    fig, axes = plt.subplots(1, len(structures), figsize=(18, 6), sharey=True, constrained_layout=True)
    if len(structures) == 1:
        axes = [axes]

    colors = plt.cm.tab10.colors
    linestyles = {"int": "-", "string": "--"}

    for ax, structure in zip(axes, structures):
        subset = grouped[grouped["structure"] == structure]
        for index, ((collision, key_type), series) in enumerate(subset.groupby(["collision", "key_type"])):
            series = series.sort_values("n_tweets")
            label = f"{collision} / {key_type}"
            ax.errorbar(
                series["n_tweets"],
                series["mean_time_ms"],
                yerr=series["std_time_ms"].fillna(0.0),
                marker="o",
                linewidth=2,
                capsize=3,
                linestyle=linestyles.get(key_type, "-"),
                color=colors[index % len(colors)],
                label=label,
            )
        ax.set_title(structure)
        ax.set_xlabel("Tweets almacenados")
        ax.grid(True, alpha=0.3)
        ax.legend(fontsize=8)

    axes[0].set_ylabel("Tiempo promedio de creación (ms)")
    fig.suptitle("Rendimiento en la creación de la estructura de datos", fontsize=16, fontweight="bold")
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=200, bbox_inches="tight")
    plt.close(fig)


def main() -> None:
    args = parse_args()
    csv_path = Path(args.input)
    output_dir = Path(args.output_dir)

    df = load_data(csv_path)
    print_statistics(df)

    report_output = output_dir / "benchmark_report.txt"
    memory_output = output_dir / "benchmark_comparacion_memoria.png"
    scalability_output = output_dir / "benchmark_escalabilidad_creacion.png"
    write_text_report(df, report_output)
    plot_comparacion_memoria(df, memory_output)
    plot_escalabilidad_creacion(df, scalability_output)

    print(f"\nTXT guardado en: {report_output}")
    print(f"\nPNG guardado en: {memory_output}")
    print(f"PNG guardado en: {scalability_output}")


if __name__ == "__main__":
    main()