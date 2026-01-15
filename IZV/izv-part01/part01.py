"""IZV cast1 projektu 2025.

Autor: Peter Stáhl xstahl01

Detailni zadani projektu je v samostatnem projektu e-learningu.
Nezapomente na to, ze python soubory maji dane formatovani.

Muzete pouzit libovolnou vestavenou knihovnu a knihovny predstavene
na prednasce
"""

from dataclasses import dataclass
from itertools import islice
from typing import Any, Dict, List

import matplotlib.pyplot as plt
import numpy as np
import requests
from bs4 import BeautifulSoup
from matplotlib.axes import Axes
from matplotlib.ticker import FuncFormatter, MultipleLocator
from numpy.typing import NDArray

# ============================================================================
# Public functions
# ============================================================================


def wave_inference_bad(
    x: NDArray[any], y: NDArray[any], sources: NDArray[any], wavelength: float
) -> NDArray[any]:
    """Referencni implementace, ktera je pomala a nevyuziva numpy efektivne; nezasahujte do ni!."""
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
    """Compute wave interference pattern efficiently using NumPy broadcasting.

    Args:
        x (NDArray[any]): 1D array of x-coordinates
        y (NDArray[any]): 1D array of y-coordinates
        sources (NDArray[any]): 2D array of shape (Ns, 2) with source positions
        wavelength (float): Wavelength for the interference calculation

    Returns:
        NDArray[any]: 2D array of shape (len(y), len(x)) with computed wave amplitudes

    """
    X, Y = np.meshgrid(x, y)
    k = 2 * np.pi / wavelength

    # Expanding dimensions for broadcasting over sources
    X_exp = X[..., np.newaxis]  # (Ny, Nx, 1)
    Y_exp = Y[..., np.newaxis]

    sx = sources[:, 0]
    sy = sources[:, 1]

    # Compute distance tensor (Ny, Nx, Ns)
    R = np.sqrt((X_exp - sx) ** 2 + (Y_exp - sy) ** 2)
    # Compute amplitude and reduce over sources
    return np.sum(np.cos(k * R) / (1 + R), axis=2)


def plot_wave(
    Z: NDArray[any],
    x: NDArray[any],
    y: NDArray[any],
    show_figure: bool = False,
    save_path: str | None = None,
) -> None:
    """Plot the wave interference pattern.

    Args:
        Z (NDArray[any]): 2D array of wave amplitudes
        x (NDArray[any]): 1D array of x-coordinates
        y (NDArray[any]): 1D array of y-coordinates
        show_figure (bool, optional): Whether to display the figure. Defaults to False.
        save_path (str | None, optional): Path to save the figure. Defaults to None.

    """
    plt.figure(figsize=(6, 5))
    img = plt.imshow(
        Z,
        extent=[x.min(), x.max(), y.min(), y.max()],
        origin="lower",
        cmap="viridis",
        aspect="equal",
        vmin=-1.0,
        vmax=1.0,
    )
    plt.xlabel("X pozice")
    plt.ylabel("Y pozice")
    plt.title("Vlnové pole")
    plt.colorbar(img, label="Amplitudy vlny")
    plt.tight_layout()

    _save_or_show_figure(show_figure, save_path)


def generate_sinus(show_figure: bool = False, save_path: str | None = None) -> None:
    """Generate and plot sinus and cosinus functions with filled areas and min/max highlighting.

    Args:
        show_figure (bool, optional): Whether to display the figure. Defaults to False.
        save_path (str | None, optional): Path to save the figure. Defaults to None.

    """
    x = np.linspace(0, 4 * np.pi, 400)
    sin_y = np.sin(x)
    cos_y = np.cos(x)

    _, axes = plt.subplots(2, 1, sharex=True, sharey=True, figsize=(8, 6))

    ax1: Axes = axes[0]
    ax2: Axes = axes[1]

    _setup_shared_axes(axes, xlim=(0, 4 * np.pi), ylim=(-1.5, 1.5), ylabel="f(x)")

    # --- Top subplot ---
    _plot_sin_cos_with_fill(ax1, x, sin_y, cos_y)

    # --- Bottom subplot ---
    _plot_min_max(ax2, x, sin_y, cos_y)

    # --- π/2 tick formatting ---
    _setup_pi_ticks(ax2)

    plt.tight_layout()

    _save_or_show_figure(show_figure, save_path)


def download_data() -> Dict[str, List[Any]]:
    """Download and parse geographic data from a specified URL.

    Returns:
        Dict[str, List[Any]]: Dictionary with keys containing lists of data about weather stations:

    """
    url = "https://ehw.fit.vutbr.cz/izv/st_zemepis_cz.html"  # URL to fetch data from
    table_attrs = {"width": "100%"}  # Attributes to identify the correct table

    # originally i had this, but then realized it was unnecessary to store intermediate results
    # html = _fetch_html(url)
    # valid_rows = _parse_table(html, table_attrs)
    # positions, lats, longs, heights = map(list, zip(*valid_rows, strict=False))

    positions, lats, longs, heights = map(
        list, zip(*_parse_table(_fetch_html(url), table_attrs), strict=False)
    )
    return {"positions": positions, "lats": lats, "longs": longs, "heights": heights}


# ============================================================================
# Private helper functions
# ============================================================================


# plot_wave helpers + generate_sinus helpers
def _save_or_show_figure(
    show_figure: bool = False, save_path: str | None = None
) -> None:
    """Save or show the current matplotlib figure based on parameters.

    Args:
        show_figure (bool, optional): Whether to display the figure. Defaults to False.
        save_path (str | None, optional): Path to save the figure. Defaults to None.

    """
    if save_path:
        plt.savefig(save_path, dpi=300)
    if show_figure:
        plt.show()
    plt.close()


# generate_sinus helpers
def _setup_shared_axes(
    axes: List[Axes], xlim: tuple, ylim: tuple, ylabel: str | None = None
) -> None:
    """Configure shared axes for a list of Axes.

    Args:
        axes (List[Axes]): list of Axes to configure
        xlim (tuple): limits for x-axis
        ylim (tuple): limits for y-axis
        ylabel (str | None, optional): label for y-axis. Defaults to None.

    """
    for ax in axes:
        ax.set_xlim(xlim)
        ax.set_ylim(ylim)
        if ylabel:
            ax.set_ylabel(ylabel)


def _plot_sin_cos_with_fill(
    ax: Axes,
    x: NDArray,
    sin_y: NDArray,
    cos_y: NDArray,
    line_color: str = "gray",
    fill_color: str = "green",
) -> None:
    """Plot sin and cos functions with filled area between them.

    Args:
        ax (Axes): Axes to plot on
        x (NDArray): x values
        sin_y (NDArray): sin(x) values
        cos_y (NDArray): cos(x) values
        line_color (str, optional): Color of function. Defaults to "gray".
        fill_color (str, optional): Color between the lines. Defaults to "green".

    """
    ax.plot(x, sin_y, label=r"$\sin(x)$", color=line_color)
    ax.plot(x, cos_y, label=r"$\cos(x)$", color=line_color)
    ax.fill_between(x, sin_y, cos_y, color=fill_color, alpha=0.2)
    _configure_grid(ax)


def _configure_grid(ax: Axes, linestyle: str = "--", alpha: float = 0.6) -> None:
    """Configure grid on the given Axes.

    Args:
        ax (Axes): Axes to configure
        linestyle (str, optional): Style of line. Defaults to "--".
        alpha (float, optional): Value of transparency. Defaults to 0.6.

    """
    ax.grid(True, linestyle=linestyle, alpha=alpha)  # this maybe an overkill


def _plot_min_max(
    ax: Axes,
    x: NDArray,
    sin_y: NDArray,
    cos_y: NDArray,
    sin_color: str = "tab:blue",
    cos_color: str = "tab:orange",
) -> None:
    """Plot min and max of sin and cos functions with highlighted max areas.

    Args:
        ax (Axes): Axes to plot on
        x (NDArray): x values
        sin_y (NDArray): sin(x) values
        cos_y (NDArray): cos(x) values
        sin_color (_type_, optional): Color of sinnus function. Defaults to "tab:blue".
        cos_color (_type_, optional): Color of cosinnus function. Defaults to "tab:orange".

    """
    min_y = np.minimum(sin_y, cos_y)
    max_y = np.maximum(sin_y, cos_y)

    ax.plot(x, min_y, linestyle="--", color="gray", linewidth=1.2)

    mask_cos = cos_y > sin_y
    mask_sin = ~mask_cos

    # Masked arrays prevent connecting lines across gaps
    ax.plot(x, np.ma.masked_where(~mask_cos, max_y), color=cos_color, linewidth=1.4)
    ax.plot(x, np.ma.masked_where(~mask_sin, max_y), color=sin_color, linewidth=1.4)

    _configure_grid(ax)


def _create_pi_formatter() -> FuncFormatter:
    """Create formatter for axis ticks as multiples of pi/2.

    Returns:
        FuncFormatter: Formatter for axis ticks

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


def _setup_pi_ticks(ax: Axes, pi_interval: float = np.pi / 2) -> None:
    """Set up x-axis ticks at multiples of pi with appropriate formatting.

    Args:
        ax (Axes): Axes to configure
        pi_interval (float, optional): Interval between ticks in radians. Defaults to np.pi/2.

    """
    ax.xaxis.set_major_locator(MultipleLocator(pi_interval))
    ax.xaxis.set_major_formatter(_create_pi_formatter())


# download_data helpers
@dataclass
class _GeoDataIndices:
    """Column indices for geographic data extraction."""

    POSITION: int = 0
    LATITUDE: int = 2
    LONGITUDE: int = 4
    HEIGHT: int = 6
    MIN_COLUMNS: int = 7


def _fetch_html(url: str, timeout: int = 10) -> str:
    """Download HTML content from a URL.

    Args:
        url (str): URL to download data from
        timeout (int, optional): Timeout for the request in seconds. Defaults to 10.

    Raises:
        RuntimeError: If the request fails or returns a non-200 status code

    Returns:
        str: HTML content of the page

    """
    try:
        response = requests.get(url, timeout=timeout)
        response.raise_for_status()
        response.encoding = response.apparent_encoding
        return response.text
    except requests.RequestException as e:
        raise RuntimeError(f"Failed to download data from {url}") from e


def _parse_table(html: str, table_attrs: dict[str, str]) -> List[tuple]:
    """Parse the HTML table and extract geographic data.

    Args:
        html (str): HTML content to parse
        table_attrs (dict[str, str]): Attributes to identify the correct table

    Raises:
        ValueError: If the table cannot be found in the HTML

    Returns:
        List[tuple]: List of tuples with extracted data (position, latitude, longitude, height)

    """
    soup = BeautifulSoup(html, "html.parser")
    table = soup.find("table", attrs=table_attrs)
    if not table:
        raise ValueError("Could not find the data table in the HTML")

    indices = _GeoDataIndices()
    rows = table.select("tr")  # CSS selector, should be slightly faster

    # rows = table.find_all('tr')

    # original implementation with intermediate generator
    # data_rows = (
    #     _extract_row_data(row.find_all('td'), indices)
    #     for row in islice(rows, 1, None)
    # )
    # return list(filter(None, data_rows))

    # Single-pass filtering with walrus operator, which in theory should be slightly faster
    return [
        data
        for row in islice(rows, 1, None)  # skip header row
        if (data := _extract_row_data(row.find_all("td"), indices)) is not None
    ]


def _parse_numeric_value(value: str) -> float:
    """Parse and clean numeric geographic values.

    Args:
        value (str): Raw string value to parse

    Returns:
        float: Parsed numeric value

    """
    return float(value.replace("°", "").replace(",", ".").replace("\xa0", "").strip())


def _extract_row_data(
    cols: List[Any], indicies: _GeoDataIndices
) -> tuple[str, float, float, float] | None:
    """Extract and validate data from a table row.

    Args:
        cols (List[Any]): List of column elements in the row
        indicies (_GeoDataIndices): Indices for relevant columns

    Returns:
        tuple[str, float, float, float] | None: Tuple with (position, latitude, longitude, height) or None if invalid

    """
    if len(cols) < indicies.MIN_COLUMNS:
        return None
    position_elem = cols[indicies.POSITION].find("strong")
    if not position_elem:
        return None
    try:
        position = position_elem.get_text(strip=True)
        lat = _parse_numeric_value(cols[indicies.LATITUDE].get_text(strip=True))
        long = _parse_numeric_value(cols[indicies.LONGITUDE].get_text(strip=True))
        height = _parse_numeric_value(cols[indicies.HEIGHT].get_text(strip=True))
        return position, lat, long, height
    except (ValueError, AttributeError):
        return None


if __name__ == "__main__":
    try:
        # X = np.linspace(-10, 10, 200)
        # Y = np.linspace(-10, 10, 200)
        # S = np.array([[-3, 0], [3, 0], [0, 4]])
        # Z1 = wave_inference_bad(X, Y, S, 2)
        # Z2 = wave_inference(X, Y, S, 2)

        # print("Equal within tolerance:", np.allclose(Z1, Z2))
        # plot_wave(Z2, X, Y, show_figure=True)
        # generate_sinus(show_figure=True)
        # download_data()
        print(download_data())
    except KeyboardInterrupt:
        print("")
