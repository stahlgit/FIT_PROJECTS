"""Peter Stáhl (xstah01)."""
#!/usr/bin/env python3
# coding=utf-8

import re
import time
import zipfile
from pathlib import Path

import matplotlib.dates as mdates
import pandas as pd  # type: ignore
import seaborn as sns  # type: ignore
from lxml import html  # type: ignore
from matplotlib import pyplot as plt

# muzete pridat libovolnou zakladni knihovnu ci knihovnu predstavenou na prednaskach
# dalsi knihovny pak na dotaz

# === GLOBAL CONSTANTS ===
REGION_ORDER = ["PHA", "STC", "JHC", "PLK", "ULK", "HKK", "JHM", "MSK", "OLK", "ZLK", "VYS", "PAK", "LBK", "KVK"]

# Mapping for Task 2
REGION_MAP = {
    0: "PHA",
    1: "STC",
    2: "JHC",
    3: "PLK",
    4: "ULK",
    5: "HKK",
    6: "JHM",
    7: "MSK",
    14: "OLK",
    15: "ZLK",
    16: "VYS",
    17: "PAK",
    18: "LBK",
    19: "KVK",
}

# Mapping for Task 3 (p57)
# Note: Code 3 (drugs) excluded based on assignment visual analysis
STATE_MAP = {
    "4": "Stav řidiče: pod vlivem alkoholu do 0,99 ‰",
    "5": "Stav řidiče: pod vlivem alkoholu 1 ‰ a více",
    "6": "Stav řidiče: nemoc, úraz apod.",
    "7": "Stav řidiče: invalida",
    "8": "Stav řidiče: řidič při jízdě zemřel",
    "9": "Stav řidiče: sebevražda",
}

# Mapping for Task 4 (p59g)
INJURY_MAP = {
    1: "Následky nehody: Usmrcení",
    2: "Následky nehody: Těžké zranění",
    3: "Následky nehody: Lehké zranění",
    4: "Následky nehody: Bez zranění",
}

# Mapping for Task 5 (p18)
CONDITION_MAP = {1: "neztížené", 2: "mlha", 3: "na počátku deště", 4: "déšť", 5: "sněžení", 6: "náledí", 7: "vítr"}


# === HELPER FUNCTIONS ===
def _check_required_columns(df: pd.DataFrame, columns: list[str]) -> None:
    """Check if the DataFrame contains specific columns.

    Args:
        df (pd.DataFrame): The DataFrame to check.
        columns (list[str]): List of required column names.

    """
    missing = [c for c in columns if c not in df.columns]
    if missing:
        raise ValueError(f"Missing columns: {missing}")


def _finalize_plot(
    g: sns.FacetGrid, title_attr: str, fig_location: str | None, show_figure: bool, colour: str = "#EAEAF2"
) -> None:
    """Routine to style, save, and show plots.

    Args:
        g (sns.FacetGrid): The seaborn FacetGrid object to finalize.
        title_attr (str): Title attribute for the plot.
        fig_location (str | None): File path to save the figure, or None to skip saving.
        show_figure (bool): Whether to display the figure.
        colour (str): Background color for the plot.

    """
    # Set titles and axis labels based on the specific plot type
    if title_attr:
        g.set_titles(title_attr)

    # Unified styling for background
    for ax in g.axes.flat:
        ax.set_facecolor(colour)

    plt.tight_layout()
    if g._legend:
        g._legend.set_bbox_to_anchor((1.0, 0.5))  # Anchor to the right edge
        g.fig.subplots_adjust(right=0.85)  # Reserve 15% of width for legend

    # Saving logic
    if fig_location:
        folder = Path(fig_location).parent
        if not folder.exists() and str(folder) != ".":
            folder.mkdir(parents=True, exist_ok=True)
        plt.savefig(fig_location)

    # Display logic
    if show_figure:
        plt.show()


# Ukol 1: nacteni dat ze ZIP souboru
def load_data(filename: str, ds: str) -> pd.DataFrame:
    """Load data from a ZIP file containing Excel files for specified years and dataset identifier.

    Args:
        filename (str): Path to the ZIP file.
        ds (str): Dataset identifier to filter files within the ZIP.

    Returns:
        pd.DataFrame: Concatenated DataFrame of all matching Excel files.

    """
    if not Path(filename).exists():
        raise FileNotFoundError(f"File {filename} not found.")
    years = [2023, 2024, 2025]
    dfs = []

    with zipfile.ZipFile(filename, "r") as zf:
        files = zf.namelist()

        for year in years:
            # pick matching file
            name = next((f for f in files if f.endswith(".xls") and ds in f and str(year) in f), None)
            if not name:
                continue

            raw: bytes = zf.read(name)
            html_text: str = raw.decode("cp1250", errors="ignore")

            # fastest table isolation - lxml + regex
            m = re.search(r"<table.*?>.*?</table>", html_text, flags=re.S | re.I)
            if not m:
                continue
            table_html = m.group(0)

            # fast table parsing
            tree = html.fromstring(table_html)
            rows = [[cell.text_content() for cell in row.xpath(".//td|.//th")] for row in tree.xpath("//tr")]

            header, body = rows[0], rows[1:]
            H = len(header)
            normalized = []
            for row in body:
                if len(row) < H:
                    row = row + [""] * (H - len(row))
                elif len(row) > H:
                    row = row[:H]
                normalized.append(row)

            df = pd.DataFrame(normalized, columns=header)

            df = df.loc[:, ~df.columns.str.contains("^Unnamed")]
            df = df.astype("string")

            dfs.append(df)

    return pd.concat(dfs, ignore_index=True) if dfs else pd.DataFrame()


# LXML : Data loaded in 37.42 seconds
# Pandas read_html: Data loaded (method 2) in 80.99 seconds
def load_data_pandas(filename: str, ds: str) -> pd.DataFrame:
    """Load data from a ZIP file containing Excel files for specified years and dataset identifier using pandas."""
    if not Path(filename).exists():
        raise FileNotFoundError(f"File {filename} not found.")
    years = [2023, 2024, 2025]
    dfs = []

    with zipfile.ZipFile(filename, "r") as zf:
        files = zf.namelist()

        for year in years:
            name = next((f for f in files if f.endswith(".xls") and ds in f and str(year) in f), None)
            if not name:
                continue

            raw = zf.read(name)
            html_text = raw.decode("cp1250", errors="ignore")

            try:
                tables = pd.read_html(html_text, encoding="cp1250")
            except ValueError:
                continue

            if tables:
                df = tables[0]

                df = df.loc[:, ~df.columns.astype(str).str.contains("^Unnamed")]
                df = df.astype("string")
                dfs.append(df)

    return pd.concat(dfs, ignore_index=True)


# Ukol 2: zpracovani dat
def parse_data(df: pd.DataFrame, verbose: bool = False) -> pd.DataFrame:
    """Clean and format the accident data.

    Args:
        df (pd.DataFrame): Raw accident data.
        verbose (bool): Whether to print memory usage information.

    """
    new_df = df.copy()

    # Date conversion
    if "p2a" in new_df.columns:
        new_df["date"] = pd.to_datetime(new_df["p2a"], dayfirst=True, errors="coerce").dt.date

    # Region mapping
    if "p4a" in new_df.columns:
        new_df["region"] = pd.to_numeric(new_df["p4a"], errors="coerce").map(REGION_MAP)

    # Drop duplicates
    if "p1" in new_df.columns:
        new_df["p1"] = new_df["p1"].astype(str)
        new_df = new_df.drop_duplicates(subset="p1")

    # Verbose output
    if verbose:
        size_mb = new_df.memory_usage(deep=True).sum() / 1e6
        print(f"new size = {size_mb:.1f} MB")

    return new_df


# Ukol 3: počty nehod v jednotlivých regionech podle stavu řidiče
def plot_state(df: pd.DataFrame, df_vehicles: pd.DataFrame, fig_location: str | None = None, show_figure: bool = False):
    """Visualize number of accidents by driver state and region.

    Args:
        df (pd.DataFrame): DataFrame containing accident data.
        df_vehicles (pd.DataFrame): DataFrame containing vehicle data.
        fig_location (str | None): File path to save the figure, or None to skip saving.
        show_figure (bool): Whether to display the figure.

    """
    _check_required_columns(df, ["p1", "region"])
    _check_required_columns(df_vehicles, ["p1", "p57"])

    # Merge and Filter
    merged = df.merge(df_vehicles, on="p1", how="inner")
    data = merged[merged["p57"].isin(STATE_MAP.keys())].copy()
    data["state_desc"] = data["p57"].map(STATE_MAP)

    # Aggregate
    agg = data.groupby(["region", "state_desc"])["p1"].nunique().reset_index(name="count")

    # Plot
    sns.set_style("ticks")

    g = sns.catplot(
        data=agg,
        x="region",
        y="count",
        col="state_desc",
        kind="bar",
        col_wrap=2,
        sharey=False,
        sharex=True,
        hue="region",
        palette="Set2",  # Hopefully i could just use some preset + custom background >w<
        legend=False,
        order=REGION_ORDER,
        height=4,
        aspect=1.5,
    )

    g.set_axis_labels("Kraj", "Počet nehod")
    g.set_titles("{col_name}")

    _finalize_plot(g, "Graf stavov vodiča", fig_location, show_figure, "#F5DCA7")


# Ukol4: alkohol a roky v krajích
def plot_alcohol(
    df: pd.DataFrame, df_consequences: pd.DataFrame, fig_location: str | None = None, show_figure: bool = False
):
    """Visualize alcohol-related accidents by consequences and region.

    Args:
        df (pd.DataFrame): DataFrame containing accident data.
        df_consequences (pd.DataFrame): DataFrame containing consequences data.
        fig_location (str | None): File path to save the figure, or None to skip saving.
        show_figure (bool): Whether to display the figure.

    """
    _check_required_columns(df, ["p1", "p11", "date", "region"])
    _check_required_columns(df_consequences, ["p1", "p59g"])

    # Prepare data
    merged = pd.merge(df, df_consequences, on="p1", how="inner")
    merged["date"] = pd.to_datetime(merged["date"], errors="coerce")

    # Filter: Month <= 10 and Alcohol >= 3
    mask = (merged["date"].dt.month <= 10) & (pd.to_numeric(merged["p11"], errors="coerce") >= 3)
    data = merged[mask].copy()

    # Map consequences
    data["p59g"] = pd.to_numeric(data["p59g"], errors="coerce")
    data = data[data["p59g"].isin(INJURY_MAP.keys())]
    data["injury_desc"] = data["p59g"].map(INJURY_MAP)

    # Aggregate
    agg = data.groupby(["region", data["date"].dt.year.rename("year"), "injury_desc"]).size().reset_index(name="count")
    agg["year"] = agg["year"].astype(int)

    # Plot
    sns.set_style("ticks")
    g = sns.catplot(
        data=agg,
        x="region",
        y="count",
        hue="year",
        col="injury_desc",
        col_wrap=2,
        kind="bar",
        sharey=False,
        sharex=True,
        height=4,
        aspect=1.5,
        order=REGION_ORDER,
        palette="viridis",
        legend=True,
    )

    # Grid lines
    for ax in g.axes.flat:
        ax.yaxis.grid(True, color="white")
        ax.set_axisbelow(True)

    g.set_axis_labels("Kraj", "Počet nehod pod vlivem")
    g._legend.set_title("Rok")

    _finalize_plot(g, "{col_name}", fig_location, show_figure)


# Ukol 5: Podmínky v čase
def plot_conditions(df: pd.DataFrame, fig_location: str | None = None, show_figure: bool = False) -> None:
    """Visualize accident counts over time by weather conditions and region.

    Args:
        df (pd.DataFrame): DataFrame containing accident data.
        fig_location (str | None): File path to save the figure, or None to skip saving.
        show_figure (bool): Whether to display the figure.

    """
    _check_required_columns(df, ["p1", "p18", "date", "region"])

    # Filter Regions and Conditions
    data = df[df["region"].isin(["JHM", "MSK", "OLK", "ZLK"])].copy()
    data["p18"] = pd.to_numeric(data["p18"], errors="coerce")
    data = data[data["p18"].isin(CONDITION_MAP.keys())]
    data["condition"] = data["p18"].map(CONDITION_MAP)
    data["date"] = pd.to_datetime(data["date"], errors="coerce")

    # Pivot -> Resample -> Stack
    pivot = data.pivot_table(index="date", columns=["region", "condition"], values="p1", aggfunc="count", fill_value=0)
    long_df = (
        pivot.resample("MS").sum().stack(level=["region", "condition"], future_stack=True).reset_index(name="count")
    )

    # Time filter
    long_df = long_df[(long_df["date"] >= "2023-01-01") & (long_df["date"] < "2025-01-01")]

    # Plot
    sns.set_style("ticks")
    g = sns.relplot(
        data=long_df,
        x="date",
        y="count",
        hue="condition",
        col="region",
        kind="line",
        col_wrap=2,
        height=4,
        aspect=1.5,
        palette="deep",
        facet_kws={"sharey": False, "sharex": True},
    )

    quarter_locator = mdates.MonthLocator(bymonth=[1, 4, 7, 10])
    date_formatter = mdates.DateFormatter("%m/%y")

    for ax in g.axes.flat:
        ax.yaxis.grid(True, color="white")

        ax.xaxis.set_major_locator(quarter_locator)
        ax.xaxis.set_major_formatter(date_formatter)
        ax.xaxis.grid(True, color="white")

        plt.setp(ax.get_xticklabels(), rotation=45, ha="right")

        ax.set_axisbelow(True)

    g.set_axis_labels("", "Počet nehod")
    g._legend.set_title("Podmínky")

    _finalize_plot(g, "Kraj: {col_name}", fig_location, show_figure)


def test_loading_speed() -> None:
    """Test and print loading speed for both data loading methods."""
    start_time = time.time()
    load_data("data_23_25.zip", "nehody")
    print(f"Data loaded in {time.time() - start_time:.2f} seconds")
    start_time = time.time()
    load_data_pandas("data_23_25.zip", "nehody")
    print(f"Data loaded (method 2) in {time.time() - start_time:.2f} seconds")


if __name__ == "__main__":
    # zde je ukazka pouziti, tuto cast muzete modifikovat podle libosti
    # skript nebude pri testovani pousten primo, ale budou volany konkreni
    # funkce.

    # test_loading_speed()

    df = load_data("data_23_25.zip", "nehody")
    df_consequences = load_data("data_23_25.zip", "nasledky")
    df_vehicles = load_data("data_23_25.zip", "Vozidla")
    df2 = parse_data(df, True)

    plot_state(df2, df_vehicles, "01_state.png")
    plot_alcohol(df2, df_consequences, "02_alcohol.png")
    plot_conditions(df2, "03_conditions.png")

# Poznamka:
# pro to, abyste se vyhnuli castemu nacitani muzete vyuzit napr
# VS Code a oznaceni jako bunky (radek #%%% )
# Pak muzete data jednou nacist a dale ladit jednotlive funkce
# Pripadne si muzete vysledny dataframe ulozit nekam na disk (pro ladici
# ucely) a nacitat jej naparsovany z disku
