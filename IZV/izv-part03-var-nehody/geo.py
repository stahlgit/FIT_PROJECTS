"""Úkol 1: Vizualizace geografických dat."""

import contextlib
from pathlib import Path

import contextily as cx
import geopandas
import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
from sklearn.cluster import DBSCAN, MiniBatchKMeans

REGION_MAP: dict[int, str] = {
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


def save_fig(fig_location: str | None = None, show_figure: bool = False):
    """Uloží alebo zobrazí aktuálnu figúru matplotlib.

    Args:
        fig_location (str | None): Cesta na uloženie obrázku. Ak je None, obrázok sa neuloží.
        show_figure (bool): Ak je True, zobrazí sa obrázok.
    """
    plt.tight_layout()

    if fig_location:
        path = Path(fig_location)
        path.parent.mkdir(parents=True, exist_ok=True)
        plt.savefig(path)

    if show_figure:
        plt.show()
        return
    plt.close()


def make_geo(df_accidents: pd.DataFrame, df_locations: pd.DataFrame) -> geopandas.GeoDataFrame:
    """Vytvára GeoDataFrame spojením dát o nehodách a ich lokalitách.

    Args:
        df_accidents (pd.DataFrame): DataFrame obsahujúci dáta o nehodách.
        df_locations (pd.DataFrame): DataFrame obsahujúci dáta o lokalitách.

    Returns:
        geopandas.GeoDataFrame: Spojený GeoDataFrame s geometriou.
    """
    # Konvertovanie dataframe do geopandas.GeoDataFrame so správnym kódovaním
    df_merged = (df_accidents.merge(df_locations, on="p1", how="inner")).dropna(subset=["d", "e"])

    if "p2a" in df_merged.columns:
        df_merged["date"] = pd.to_datetime(df_merged["p2a"], format="%d.%m.%Y", errors="coerce")

    df_merged = df_merged[(df_merged["d"] != 0) & (df_merged["e"] != 0)]

    mask = df_merged["d"] < df_merged["e"]
    if mask.any():
        df_merged.loc[mask, ["d", "e"]] = df_merged.loc[mask, ["e", "d"]].values
    return geopandas.GeoDataFrame(
        df_merged, geometry=geopandas.points_from_xy(df_merged["d"], df_merged["e"]), crs="EPSG:5514"
    )


def plot_geo(
    gdf: geopandas.GeoDataFrame, fig_location: str | None = None, show_figure: bool = False, region_id: int = 6
) -> None:
    """Vykreslí graf nehôd so zverou pre daný kraj.

    Args:
        gdf (geopandas.GeoDataFrame): GeoDataFrame s dátami o nehodách.
        fig_location (str | None): Cesta na uloženie obrázku. Ak je None, obrázok sa neuloží.
        show_figure (bool): Ak je True, zobrazí sa obrázok.
        region_id (int): ID kraja podľa REGION_MAP.

    Raises:
        ValueError: Ak region_id nie je v REGION_MAP.
    """
    if region_id not in REGION_MAP:
        raise ValueError(f"Region ID {region_id} nie je v zozname známych krajov.")
    # Vykresleni grafu s nehodami se zvěří pro roky 2023-2024
    # p4a = kraj
    # p10 = nehoda se zvěří
    type_of_incident = 4
    gdf_filtered: geopandas.GeoDataFrame = gdf[(gdf["p4a"] == region_id) & (gdf["p10"] == type_of_incident)].copy()
    region_name = REGION_MAP[region_id]
    if gdf_filtered.empty:
        print(f"Nenalezeny žádné nehody se zvěří pro kraj {region_name}.")
        return

    # Převod do projekce Web Mercator pro kompatibilitu s podkladovými mapami
    gdf_3857 = gdf_filtered.to_crs(epsg=3857)

    fig, axes = plt.subplots(1, 2, figsize=(14, 6), sharex=True, sharey=True)
    fig.suptitle(f"Nehody zaviněné zvěří v {region_name} kraji (2023 vs 2024)", fontsize=16)

    years = [2023, 2024]

    # Vykreslení pro každý rok
    for i, year in enumerate(years):
        ax = axes[i]

        gdf_year = gdf_3857[gdf_3857["date"].dt.year == year]

        if not gdf_year.empty:
            gdf_year.plot(ax=ax, markersize=3, color="red", alpha=0.6, label="Nehoda")
            
            # Podkladová mapa
            with contextlib.suppress(Exception):
                cx.add_basemap(ax, crs=gdf_year.crs.to_string(), source=cx.providers.OpenStreetMap.Mapnik, alpha=0.9)
        ax.set_title(f"{region_name} kraj ({year})")
        ax.axis("off")
    save_fig(fig_location, show_figure)


def plot_cluster(
    gdf: geopandas.GeoDataFrame, fig_location: str | None = None, show_figure: bool = False, region_id: int = 6
) -> None:
    """Vykreslenie grafu s lokalitou všetkých nehôd v kraji zlúčených do clusterov.

    Metóda klastrovania: KMeans(MiniBatchKMeans)
    Dôvod:
        KMeans bol zovolený pre jeho rýchlosť a efektívnosť pri rozdeľovaní oblasti na 'n' zón.

    Args:
        gdf (geopandas.GeoDataFrame): GeoDataFrame s dátami o nehodách.
        fig_location (str | None): Cesta na uloženie obrázku. Ak je None, obrázok sa neuloží.
        show_figure (bool): Ak je True, zobrazí sa obrázok.
        region_id (int): ID kraja podľa REGION_MAP.

    Raises:
        ValueError: Ak region_id nie je v REGION_MAP.
    """
    if region_id not in REGION_MAP:
        raise ValueError(f"Region ID {region_id} nie je v zozname známych krajov.")

    signification_alcohol_level = 4
    gdf_filtered = gdf[(gdf["p4a"] == region_id) & (gdf["p11"] >= signification_alcohol_level)].copy()
    if gdf_filtered.empty:
        print(f"Pro kraj {REGION_MAP[region_id]} nebyla nalezena data s vlivem alkoholu.")
        return

    gdf_3857 = gdf_filtered.to_crs(epsg=3857)

    # Extrakcia súradníc (Optimalizované cez numpy)
    coords = np.array(list(zip(gdf_3857.geometry.x, gdf_3857.geometry.y, strict=False)))

    ### --- Filtering outliers (DBSCAN) ---
    db = DBSCAN(eps=40000, min_samples=3)  # 40 km (dostatečne bezpečné pre vylúčenie outlierov)
    labels = db.fit_predict(coords)

    unique_labels, counts = np.unique(labels, return_counts=True)
    valid_indices = unique_labels != -1  # -1 sú outliers
    if not any(valid_indices):
        print("Všetky body boli označené ako outliers, nie je možné pokračovať v klastrovaní.")
        return

    largest_cluster_label = unique_labels[valid_indices][counts[valid_indices].argmax()]
    gdf_3857 = gdf_3857[labels == largest_cluster_label].copy()
    if gdf_3857.empty:
        print("Po filtrácii outliers nezostali žiadne dáta.")
        return

    ### --- Clustering ---
    coords = np.array(
        list(zip(gdf_3857.geometry.x, gdf_3857.geometry.y, strict=False))
    )  # znovu extrahujeme súradnice po filtrovaní

    # heuristika: ~ 1 cluster na 20 nehôd,min 1 max 50 clusterov
    num_accidents = len(coords)
    n_clusters = max(1, num_accidents // 20)
    n_clusters = min(n_clusters, 50)

    kmeans = MiniBatchKMeans(n_clusters=n_clusters, random_state=42, n_init="auto")
    label = kmeans.fit_predict(coords)

    gdf_3857["cluster"] = label

    ### Agregace podle clusterů
    gdf_clusters = gdf_3857.dissolve(by="cluster", aggfunc="count")
    gdf_clusters = gdf_clusters.rename(columns={"p1": "count"})

    gdf_clusters["geometry"] = gdf_clusters.geometry.convex_hull

    _, ax = plt.subplots(figsize=(12, 10))
    region_name = REGION_MAP[region_id]
    ax.set_title(f"Nehody v {region_name} kraji s významnou měrou alkoholu")

    # Vykreslení clusterů (polygonů) podle četnosti
    gdf_clusters.plot(
        column="count",
        ax=ax,
        cmap="viridis",
        legend=True,
        alpha=0.5,  # Průhlednost
        legend_kwds={"label": "Počet nehod v úseku", "orientation": "horizontal", "shrink": 0.8},
    )

    # Vykreslení samotných bodů nehod (pro detail)
    gdf_3857.plot(ax=ax, markersize=1.5, color="blue", alpha=0.6)

    # Podkladová mapa
    try:
        cx.add_basemap(ax, crs=gdf_3857.crs.to_string(), source=cx.providers.OpenStreetMap.Mapnik)
    except Exception as e:
        print(f"Nelze načíst mapový podklad: {e}")

    ax.axis("off")
    save_fig(fig_location, show_figure)


if __name__ == "__main__":
    try:

        def get_dataframe(filename: str, verbose: bool = False) -> pd.DataFrame:
            """Načítá DataFrame z pickle súboru a optimalizuje jeho pamäťovú náročnosť.

            Args:
                filename (str): Cesta k pickle súboru.
                verbose (bool): Ak je True, vypíše informácie o pamäti pred a po optimalizácii.

            Returns:
                pd.DataFrame: Optimalizovaný DataFrame.
            """
            try:
                df = pd.read_pickle(filename)  # načítanie dát z pickle súboru, pandas to pozná sám
            except FileNotFoundError:
                return pd.DataFrame()  # v prípade chyby vrátime prázdny DataFrame

            orig_size = df.memory_usage(deep=True).sum() / 1024**2

            # Kategorizacia objektových stĺpcov
            for col in df.select_dtypes(include=["object"]).columns:
                if col != "p2a":  # Datum nechceme kategorizovat
                    num_unique = len(df[col].unique())
                    num_total = len(df[col])
                    if num_unique / num_total < 0.5:
                        df[col] = df[col].astype("category")

            # Výpis informácie o pamäti, ak je vyžadovaný
            if verbose:
                new_size = df.memory_usage(deep=True).sum() / 1024**2
                print(f"file={filename}: orig_size={orig_size:.1f} MB, new_size={new_size:.1f} MB")

            return df

        df_accidents = get_dataframe("accidents.pkl.gz", verbose=True)
        df_locations = get_dataframe("locations.pkl.gz", verbose=True)

        gdf = make_geo(df_accidents, df_locations)

        # Kontrola hranic (bounding box)
        # min_x, min_y, max_x, max_y = gdf.total_bounds
        # print("\nRozsah souřadnic (Bounding Box):")
        # print(f"X: {min_x:.1f} až {max_x:.1f}")
        # print(f"Y: {min_y:.1f} až {max_y:.1f}")

        plot_geo(gdf, "geo1.png", show_figure=True)
        plot_cluster(gdf, "geo2.png", show_figure=True)

    except FileNotFoundError:
        print("Chyba: Soubory nebyly nalezeny. Ujistěte se, že jsou ve stejném adresáři.")
    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(f"Nastala chyba: {e}")
