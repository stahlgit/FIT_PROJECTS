"""
IZV cast1 projektu 2025
Autor: Peter Stahl xstahl01

Detailni zadani projektu je v samostatnem projektu e-learningu.
Nezapomente na to, ze python soubory maji dane formatovani.

Muzete pouzit libovolnou vestavenou knihovnu a knihovny predstavene
na prednasce
"""

from dataclasses import dataclass
from itertools import islice
from pprint import pprint
from typing import Any, Dict, List

import matplotlib.pyplot as plt
import numpy as np
import requests
from bs4 import BeautifulSoup
from matplotlib.axes import Axes
from matplotlib.ticker import FuncFormatter, MultipleLocator
from numpy.typing import NDArray


def wave_inference_bad(
    x: NDArray[any], y: NDArray[any], sources: NDArray[any], wavelength: float
) -> NDArray[any]:
    """
    Referencni implementace, ktera je pomala a nevyuziva numpy efektivne;
    nezasahujte do ni!
    """
    k = 2 * np.pi / wavelength

    Z = np.zeros(x.shape + y.shape)
    for sx, sy in sources:
        for i in range(x.shape[0]):
            for j in range(y.shape[0]):
                R = np.sqrt((x[i] - sx) ** 2 + (y[j] - sy) ** 2)
                Z[j, i] += np.cos(k * R) / (1 + R)
    return Z


def wave_inference(
    x: NDArray[any], y: NDArray[any], sources: NDArray[any], wavelength: float
) -> NDArray[any]:
    """
    Efektivni vypocet amplitudy vlnoveho pole z vice zdroju.

    Vyuziva numpy broadcasting pro efektivni vypocet interference vln
    z vice zdroju najednou bez pouziti explicitnich cyklu.

    Args:
        x: 1D pole souradnic na ose X
        y: 1D pole souradnic na ose Y
        sources: 2D pole souradnic zdroju vlneni (N x 2)
        wavelength: Vlnova delka

    Returns:
        2D pole amplitud vlnoveho pole (shape: len(y) x len(x))
    """
    X, Y = np.meshgrid(x, y)
    k = 2 * np.pi / wavelength

    # Expanding dimensions for broadcasting over sources
    X_exp = X[..., np.newaxis]  # (Ny, Nx, 1)
    Y_exp = Y[..., np.newaxis]

    sx = sources[:, 0]
    sy = sources[:, 1]

    # Compute distance tensor (Ny, Nx, Ns)
    R = np.sqrt((X_exp - sx) ** 2 + (Y_exp + sy) ** 2)
    # Compute amplitude and reduce over sources
    return np.sum(np.cos(k * R) / (1 + R), axis=2)


def _save_or_show_figure(
    show_figure: bool = False, save_path: str | None = None
) -> None:
    """
    Pomocna funkce pro ulozeni nebo zobrazeni grafu.

    Args:
        show_figure: Pokud True, zobrazi se graf pomoci plt.show()
        save_path: Cesta pro ulozeni grafu (volitelne)
    """
    if save_path:
        plt.savefig(save_path, dpi=300, bbox_inches="tight")
    if show_figure:
        plt.show()
    plt.close()


def _configure_grid(ax: Axes, linestyle: str = "--", alpha: float = 0.6) -> None:
    """
    Nastavi mrizku pro osu grafu.

    Args:
        ax: Matplotlib Axes objekt
        linestyle: Styl cary mrizky
        alpha: Pruhlednost mrizky
    """
    ax.grid(True, linestyle=linestyle, alpha=alpha)


def plot_wave(
    Z: NDArray[any],
    x: NDArray[any],
    y: NDArray[any],
    show_figure: bool = False,
    save_path: str | None = None,
) -> None:
    """
    Vizualizace vlnoveho pole.

    Args:
        Z: 2D pole amplitud vlnoveho pole
        x: 1D pole souradnic na ose X
        y: 1D pole souradnic na ose Y
        show_figure: Pokud True, zobrazi se graf pomoci plt.show()
        save_path: Cesta pro ulozeni grafu (volitelne)
    """
    plt.figure(figsize=(6, 5))
    img = plt.imshow(
        Z,
        extent=[x.min(), x.max(), y.min(), y.max()],
        origin="lower",
        cmap="viridis",
        aspect="equal",
    )
    plt.xlabel("X pozice")
    plt.ylabel("Y pozice")
    plt.title("Vlnove pole")
    plt.colorbar(img, label="Amplituda vlny")
    plt.tight_layout()

    _save_or_show_figure(show_figure, save_path)


def _create_pi_formatter() -> FuncFormatter:
    """
    Vytvori formatter pro osu s oznacenim nasobku pi/2.

    Returns:
        FuncFormatter objekt pro matplotlib
    """

    def pi_formatter(val, _) -> str:
        """Format axis ticks as multiples of pi/2."""
        n = val / np.pi
        m = int(np.round(n * 2))

        if not np.isclose(n * 2, m, atol=1e-8):
            return ""

        if m == 0:
            return r"$0$"

        if m % 2 == 0:
            k = m // 2
            if k == 1:
                return r"$\pi$"
            return rf"${k}\pi$"

        if m == 1:
            return r"$\frac{\pi}{2}$"
        return "$" + r"\frac{" + f"{m}\\pi" + r"}{2}" + "$"

    return FuncFormatter(pi_formatter)


def _setup_shared_axes(axes: List[Axes], xlim: tuple, ylim: tuple) -> None:
    """
    Nastavi spolecne limity pro vice podgrafu.

    Args:
        axes: Seznam Axes objektu
        xlim: Limity osy X (min, max)
        ylim: Limity osy Y (min, max)
    """
    for ax in axes:
        ax.set_xlim(xlim)
        ax.set_ylim(ylim)


def _plot_sin_cos_with_fill(
    ax: Axes, x: NDArray, sin_y: NDArray, cos_y: NDArray
) -> None:
    """
    Vykresli sin a cos s vyplnenou plochou mezinimi.

    Args:
        ax: Matplotlib Axes objekt
        x: Pole hodnot na ose X
        sin_y: Pole hodnot sin(x)
        cos_y: Pole hodnot cos(x)
    """
    ax.plot(x, sin_y, label=r"$\sin(x)$", color="gray")
    ax.plot(x, cos_y, label=r"$\cos(x)$", color="gray")
    ax.fill_between(x, sin_y, cos_y, color="green", alpha=0.2)
    ax.legend(loc="upper right")
    _configure_grid(ax)


def _plot_min_max_colored(
    ax: Axes, x: NDArray, sin_y: NDArray, cos_y: NDArray, min_y: NDArray, max_y: NDArray
) -> None:
    """
    Vykresli minimum carkovane a maximum barevne podle zdroje.

    Args:
        ax: Matplotlib Axes objekt
        x: Pole hodnot na ose X
        sin_y: Pole hodnot sin(x)
        cos_y: Pole hodnot cos(x)
        min_y: Pole minimalnich hodnot
        max_y: Pole maximalnich hodnot
    """
    ax.plot(x, min_y, linestyle="--", color="gray", linewidth=1.2)

    mask_cos = cos_y > sin_y
    mask_sin = ~mask_cos

    # Masked arrays prevent connecting lines across gaps
    ax.plot(x, np.ma.masked_where(~mask_cos, max_y), color="tab:orange", linewidth=1.4)
    ax.plot(x, np.ma.masked_where(~mask_sin, max_y), color="tab:blue", linewidth=1.4)

    _configure_grid(ax)


def _setup_pi_ticks(ax: Axes, pi_interval: float = np.pi / 2) -> None:
    """
    Nastavi znacky na ose X jako nasobky pi.

    Args:
        ax: Matplotlib Axes objekt
        pi_interval: Interval mezi znackami (napr. pi/2)
    """
    ax.xaxis.set_major_locator(MultipleLocator(pi_interval))
    ax.xaxis.set_major_formatter(_create_pi_formatter())
    ax.set_xlabel(r"$x$")


def generate_sinus(show_figure: bool = False, save_path: str | None = None) -> None:
    """
    Generuje vizualizaci sinusoveho a kosinusoveho signalu.

    Vytvori dva podgrafy: prvni zobrazuje sin(x) a cos(x) s vyplnenou
    plochou mezi nimi, druhy zobrazuje minimum carkovane a maximum
    barevne rozlisene podle zdroje.

    Args:
        show_figure: Pokud True, zobrazi se graf pomoci plt.show()
        save_path: Cesta pro ulozeni grafu (volitelne)
    """
    x = np.linspace(0, 4 * np.pi, 400)
    sin_y = np.sin(x)
    cos_y = np.cos(x)

    min_y = np.minimum(sin_y, cos_y)
    max_y = np.maximum(sin_y, cos_y)

    _, axes = plt.subplots(2, 1, sharex=True, sharey=True, figsize=(8, 6))

    ax1: Axes = axes[0]
    ax2: Axes = axes[1]

    _setup_shared_axes(axes, xlim=(0, 4 * np.pi), ylim=(-1.5, 1.5))

    # Top subplot: sin and cos with filled area
    _plot_sin_cos_with_fill(ax1, x, sin_y, cos_y)

    # Bottom subplot: min dashed, max colored by source
    _plot_min_max_colored(ax2, x, sin_y, cos_y, min_y, max_y)

    # Setup pi/2 ticks on x-axis
    _setup_pi_ticks(ax2)

    plt.tight_layout()

    _save_or_show_figure(show_figure, save_path)


@dataclass
class _GeoDataIndices:
    """Column indices for geographic data extraction."""

    POSITION: int = 0
    LATITUDE: int = 2
    LONGITUDE: int = 4
    HEIGHT: int = 6
    MIN_COLUMNS: int = 7


def _parse_numeric_value(value: str) -> float:
    """
    Parsuje a cisli geograficke hodnoty.

    Args:
        value: Retezec s cislem (muze obsahovat °, carky, nbsp)

    Returns:
        Float hodnota
    """
    cleaned = value.replace("°", "").replace(",", ".")
    cleaned = cleaned.replace("\xa0", "").strip()
    return float(cleaned)


def _extract_row_data(
    cols: List[Any], indices: _GeoDataIndices
) -> tuple[str, float, float, float] | None:
    """
    Extrahuje data z radku tabulky.

    Args:
        cols: Seznam bunek radku (td elementy)
        indices: Objekt s indexy sloupcu

    Returns:
        Tuple (pozice, sirka, delka, vyska) nebo None pri chybe
    """
    if len(cols) < indices.MIN_COLUMNS:
        return None
    position_elem = cols[indices.POSITION].find("strong")
    if not position_elem:
        return None
    try:
        position = position_elem.get_text(strip=True)
        lat = _parse_numeric_value(cols[indices.LATITUDE].get_text(strip=True))
        long = _parse_numeric_value(cols[indices.LONGITUDE].get_text(strip=True))
        height = _parse_numeric_value(cols[indices.HEIGHT].get_text(strip=True))
        return position, lat, long, height
    except (ValueError, AttributeError):
        return None


def _fetch_html(url: str, timeout: int = 10) -> str:
    """
    Stahne HTML stranku z dane URL.

    Args:
        url: Adresa stranky
        timeout: Timeout pro pozadavek v sekundach

    Returns:
        HTML obsah stranky

    Raises:
        RuntimeError: Pri chybe stahovani
    """
    try:
        response = requests.get(url, timeout=timeout)
        response.raise_for_status()
        response.encoding = response.apparent_encoding
        return response.text
    except requests.RequestException as e:
        raise RuntimeError(f"Failed to download data from {url}") from e


def _parse_station_table(html: str) -> List[tuple]:
    """
    Parsuje tabulku stanic z HTML.

    Args:
        html: HTML obsah stranky

    Returns:
        Seznam tuple s daty stanic

    Raises:
        ValueError: Pokud tabulka nebyla nalezena
    """
    soup = BeautifulSoup(html, "html.parser")
    table = soup.find("table", width="100%")
    if not table:
        raise ValueError("Could not find the data table in the HTML")

    indices = _GeoDataIndices()
    rows = table.find_all("tr")

    data_rows = (
        _extract_row_data(row.find_all("td"), indices) for row in islice(rows, 2, None)
    )
    return list(filter(None, data_rows))


def download_data() -> Dict[str, List[Any]]:
    """
    Stahne a zpracuje data meteorologickych stanic z webu.

    Stahuje tabulku stanic ze stranky ehw.fit.vutbr.cz a extrahuje
    nazvy stanic, zemepisne souradnice a nadmorske vysky.

    Returns:
        Slovnik obsahujici seznamy:
        - 'positions': nazvy stanic
        - 'lats': zemepisne sirky
        - 'longs': zemepisne delky
        - 'heights': nadmorske vysky

    Raises:
        RuntimeError: Pri chybe stahovani dat
        ValueError: Pri chybe parsovani HTML
    """
    url = "https://ehw.fit.vutbr.cz/izv/stanice.html"

    html = _fetch_html(url)
    valid_rows = _parse_station_table(html)

    positions, lats, longs, heights = map(list, zip(*valid_rows, strict=False))
    return {"positions": positions, "lats": lats, "longs": longs, "heights": heights}


if __name__ == "__main__":
    try:
        # X = np.linspace(-10, 10, 200)
        # Y = np.linspace(-10, 10, 200)
        # S = np.array([[-3, 0], [3, 0], [0, 4]])
        # Z1 = wave_inference_bad(X, Y, S, 2)
        # Z2 = wave_inference(X, Y, S, 2)

        # print("Equal within tolerance:", np.allclose(Z1, Z2))
        # plot_wave(Z1, X, Y, show_figure=True)
        # generate_sinus(show_figure=True)
        pprint(download_data())
    except KeyboardInterrupt:
        print("")
