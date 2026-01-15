"""Vypracovanie pre Úskol 3: Vlastní analýza."""

import textwrap

import numpy as np
import pandas as pd
import plotly.graph_objects as go

# Mapovanie konkrétnych kódov p12 na čitateľné názvy
CAUSE_MAP = {
    100: "Nezavinená vodičom",
    201: "Nepřizpůsobení rychlosti intenzitě (hustotě) provozu",
    202: "Nepřizpůsobení rychlosti viditelnosti (mlha, soumrak, jízda v noci na tlumená světla apod.)",
    203: "Nepřizpůsobení rychlosti vlastnostem vozidla a nákladu",
    204: "Nepřizpůsobení rychlosti stavu vozovky (náledí, výtluky, bláto, mokrý povrch apod.)",
    205: "Nepřizpůsobení rychlosti dopravně technickému stavu vozovky (zatáčka, klesání, stoupání, šířka vozovky apod.)",
    206: "Překročení předepsané rychlosti stanovené pravidly",
    207: "Překročení rychlosti stanovené dopravní značkou",
    208: "Nepřizpůsobení rychlosti",
    209: "Jiný druh nepřiměřené rychlosti",
    301: "Předjíždění vpravo",
    302: "Předjíždění bez dostatečného bočního odstupu",
    303: "Předjíždění bez dostatečného rozhledu (v nepřehledné zatáčce nebo její blízkosti, před vrcholem stoupání apod.)",
    304: "Při předjíždění došlo k ohrožení protijedoucího řidiče vozidla (špatný odhad vzdálenosti potřebné k předjetí apod.)",
    305: "Při předjíždění došlo k ohrožení předjížděného řidiče vozidla (vynucené zařazení, předjížděný řidič musel prudce brzdit, měnit směr jízdy apod.)",
    306: "Předjíždění vlevo vozidla odbočujícího vlevo",
    307: "Předjíždění v místech, kde je to zakázáno dopravní značkou",
    308: "Při předjíždění byla přejeta podélná čára souvislá",
    309: "Bránění v předjíždění",
    310: "Přehlédnutí již předjíždějícího souběžně jedoucího vozidla",
    311: "Jiný druh nesprávného předjíždění",
    401: "Jízda na červenou 3-barevného semaforu",
    402: "Proti příkazu dopravní značky STŮJ DEJ PŘEDNOST",
    403: "Proti příkazu dopravní značky DEJ PŘEDNOST",
    404: "Vozidlu přijíždějícímu zprava",
    405: "Při odbočování vlevo",
    406: "Tramvaji, která odbočuje",
    407: "Protijedoucímu vozidlu při objíždění překážky",
    408: "Při zařazování do proudu jedoucích vozidel ze stanice, místa zastavení nebo stání",
    409: "Při vjíždění na silnici",
    410: "Při otáčení nebo couvání",
    411: "Při přejíždění z jednoho jízdního pruhu do druhého",
    412: "Chodci na vyznačeném přechodu",
    413: "Při odbočování vlevo",
    414: "Jiné nedání přednosti",
    501: "Jízda po nesprávné straně vozovky, vjetí do protisměru",
    502: "Vyhýbání bez dostatečného bočního odstupu (vůle)",
    503: "Nedodržení bezpečné vzdálenosti za vozidlem",
    504: "Nesprávné otáčení nebo couvání ",
    505: "Chyby při udání směru jízdy",
    506: "Bezohledná, agresivní, neohleduplná jízda",
    507: "Náhlé bezdůvodné snížení rychlosti jízdy, zabrzdění nebo zastavení ",
    508: "Řidič se plně nevěnoval řízení vozidla",
    509: "Samovolné rozjetí nezajištěného vozidla",
    510: "Vjetí na nezpevněnou komunikaci",
    511: "Nezvládnutí řízení vozidla",
    512: "Jízda (vjetí) jednosměrnou ulicí, silnicí (v protisměru)",
    513: "Nehoda v důsledku  použití (policií) prostředků k násilnému zastavení vozidla (zastavovací pásy, zábrana, vozidlo atp.)",
    514: "Nehoda v důsledku použití služební zbraně (policií)",
    515: "Nehoda při provádění služebního zákroku (pronásledování pachatele atd.)",
    516: "Jiný druh nesprávného způsobu jízdy",
    601: "Závada řízení",
    602: "Závada provozní brzdy",
    603: "Neúčinná nebo nefungující parkovací brzda",
    604: "Opotřebení běhounu pláště pod stanovenou mez",
    605: "Defekt pneumatiky způsobený průrazem nebo náhlým únikem vzduchu",
    606: "Závada osvětlovací soustavy vozidla (neúčinná, chybějící, znečištěná apod.)",
    607: "Nepřipojená nebo poškozená spojovací hadice pro bzrdění přípojného vozidla",
    608: "Nesprávné uložení nákladu",
    609: "Upadnutí, ztráta kola vozidla (i rezervního)",
    610: "Zablokování kol v důsledku mechanické závady vozidla (zadřený motor, převodovka, rozvodovka, spadlý řetěz apod.)",
    611: "Lom závěsu kola, pružiny",
    612: "Nezajištěná nebo poškozená bočnice (i u přívěsu)",
    613: "Závada závěsu pro přívěs",
    614: "Utržená spojovací hřídel",
    615: "Jiná technická závada (vztahuje se i na přípojná vozidla)",
}

good = "Dobré počasie"
bad = "Zhoršené počasie"


def load_data():
    """Načítanie predspracovaných dát zo súboru."""
    return pd.read_pickle("accidents.pkl.gz")


def plot_weather_impact_plotly(df, title, filename):
    """Vytvorenie interaktívneho Plotly grafu vplyvu počasia na príčiny nehôd."""

    def wrapper(x, width=50):
        return "<br>".join(textwrap.wrap(x, width=width))

    df_plot = df.copy()
    df_plot["pricina_nazov_wrapped"] = df_plot["pricina_nazov"].astype(str).apply(wrapper)

    # Definícia farieb
    colors = {good: "#2ecc71", bad: "#e74c3c"}

    # Vytvorenie grafu
    fig = go.Figure()

    # Pridanie stĺpcov pre každé počasie
    for weather_type in [bad, good]:
        df_weather = df_plot[df_plot["pocasie_typ"] == weather_type]

        fig.add_trace(
            go.Bar(
                y=df_weather["pricina_nazov_wrapped"],
                x=df_weather["percent"],
                name=weather_type,
                orientation="h",
                marker={"color": colors[weather_type]},
                text=[f"{val:.1f}%" for val in df_weather["percent"]],
                textposition="outside",
                hovertemplate="<b>%{y}</b><br>" + "Podiel: %{x:.1f}%<br>" + "<extra></extra>",
            )
        )

    # Nastavenie layoutu
    fig.update_layout(
        title={"text": title, "x": 0.5, "xanchor": "center", "font": {"size": 16}},
        xaxis_title="Podiel nehôd v danom počasí [%]",
        yaxis_title="",
        yaxis={"autorange": "reversed"},
        barmode="group",
        height=500,
        plot_bgcolor="white",
        hovermode="closest",
        legend={"title": None, "orientation": "h", "yanchor": "bottom", "y": 1.02, "xanchor": "right", "x": 1},
        margin={"l": 10, "r": 50, "t": 80, "b": 50},
    )

    # Gridlines
    fig.update_xaxes(showgrid=True, gridwidth=1, gridcolor="lightgray")
    fig.update_yaxes(showgrid=False)

    # Uloženie ako HTML
    fig.write_html(filename)


def print_info(table_bad, table_good, total_accidents, bad_weather_accidents, risk_ratio) -> None:
    """Vytlačenie informácií do konzoly."""
    print("--- DATA PRE TABUĽKY (Top 5 príčin) ---")
    print("\nTabuľka 1: Zhoršené počasie")
    print(table_bad.to_string())
    print("\nTabuľka 2: Dobré počasie")
    print(table_good.to_string())

    print("\n--- VYPOČÍTANÉ HODNOTY PRE TEXT ---")
    print(f"celkovy_pocet_nehod: {total_accidents}")
    print(f"pocet_nehod_zhorsene_pocasie: {bad_weather_accidents}")
    print(f"zvysene_riziko_smyku_v_zlom_pocasi: {risk_ratio:.2f}x (násobne vyššie riziko)")


def generate_html_report(df_bad, df_good, text_report):
    # 1. Pripravíme DataFrames
    df_bad.index.name = "Príčina nehody"
    df_good.index.name = "Príčina nehody"
    df_bad.columns.name = None
    df_good.columns.name = None
    df_bad = df_bad.reset_index()
    df_good = df_good.reset_index()

    # 2. Generovanie HTML
    table_bad_html = df_bad.to_html(
        classes="table table-striped table-hover text-center", index=False, float_format=lambda x: f"{x:.1f}%"
    )
    table_good_html = df_good.to_html(
        classes="table table-striped table-hover text-center", index=False, float_format=lambda x: f"{x:.1f}%"
    )

    html_content = f"""
    <!DOCTYPE html>
    <html lang="sk">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Analýza nehôd: Vplyv počasia</title>
        <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
        <style>
            body {{ background-color: #f8f9fa; padding-bottom: 50px; }}
            .container {{ margin-top: 30px; max-width: 1600px; width: 95%; }}
            .card {{ margin-bottom: 30px; box-shadow: 0 4px 6px rgba(0,0,0,0.1);}}
            .card-header {{ background-color: #343a40; color: white; font-weight: bold; }}
            .graph-container {{ width: 100%; height: auto; border-radius: 5px; }}
            h1 {{ text-align: center; margin-bottom: 40px; color: #343a40; }}
            .table-container {{ overflow-x: auto; }}
            .report-text {{ font-size: 1.1em; line-height: 1.6; text-align: justify; }}
            
            .table th:first-child {{
                text-align: left !important;
                padding-left: 1.5em;
                vertical-align: middle;
            }}
            .table th {{
                text-align: center !important;
                vertical-align: middle;
            }}
            .table td:first-child {{
                text-align: left !important;
            }}

            .dev-note {{
                display: inline-block;
                background-color: #6c757d;
                color: white;
                border-radius: 50%;
                width: 18px;
                height: 18px;
                text-align: center;
                line-height: 18px;
                font-size: 12px;
                font-weight: bold;
                cursor: help;
                position: relative;
                margin-left: 8px;
                vertical-align: middle;
            }}

            .dev-note::after {{
                content: attr(data-tooltip);
                position: absolute;
                bottom: 135%;
                left: 50%;
                transform: translateX(-50%);
                background-color: #333;
                color: #fff;
                padding: 6px 10px;
                border-radius: 4px;
                font-size: 12px;
                white-space: nowrap;
                opacity: 0;
                visibility: hidden;
                transition: 0.2s;
                pointer-events: none;
                z-index: 100;
                font-weight: normal;
                box-shadow: 0 2px 4px rgba(0,0,0,0.2);
            }}

            .dev-note::before {{
                content: "";
                position: absolute;
                bottom: 90%;
                left: 50%;
                margin-left: -5px;
                border-width: 5px;
                border-style: solid;
                border-color: #333 transparent transparent transparent;
                opacity: 0;
                visibility: hidden;
                transition: 0.2s;
            }}

            .dev-note:hover::after, .dev-note:hover::before {{
                opacity: 1;
                visibility: visible;
            }}
            
            /* Frame for Plotly graphs */
            iframe {{
                border: none;
                width: 100%;
                height: 550px;
            }}
        </style>
    </head>
    <body>

    <div class="container">
        <h1>Analýza príčin nehôd a vplyv počasia</h1>
        
        <div class="card">
            <div class="card-header bg-primary">
                Slovný komentár k analýze
            </div>
            <div class="card-body">
                <p class="report-text">{text_report}</p>
            </div>
        </div>
        
        <div class="card">
            <div class="card-header">
                1. Top 5 príčin v ZHORŠENOM počasí
            </div>
            <div class="card-body">
                <div class="row">
                    <div class="col-lg-7">
                        <h5>Grafická vizualizácia (interaktívna)</h5>
                        <div class="graph-container">
                            <iframe src="graf_top_zhorsene.html"></iframe>
                        </div>
                    </div>
                    <div class="col-lg-5">
                        <h5>
                            Detailné dáta 
                            <span class="dev-note" data-tooltip="Percentuálne hodnoty vyjadrujú podiel v rámci daného typu počasia a celkový počet nehôd v dobrom počasí je výrazne vyšší než v zlom.">?</span>
                        </h5>
                        <p class="text-muted">Tabuľka ukazuje podiel príčin v rámci daného počasia.</p>
                        <div class="table-container">
                            {table_bad_html}
                        </div>
                    </div>
                </div>
            </div>
        </div>

        <div class="card">
            <div class="card-header" style="background-color: #2ecc71;">
                2. Top 5 príčin v DOBROM počasí
            </div>
            <div class="card-body">
                <div class="row">
                    <div class="col-lg-7">
                        <h5>Grafická vizualizácia (interaktívna)</h5>
                        <div class="graph-container">
                            <iframe src="graf_top_dobre.html"></iframe>
                        </div>
                    </div>
                    <div class="col-lg-5">
                        <h5>
                            Detailné dáta
                            <span class="dev-note" data-tooltip="Percentuálne hodnoty vyjadrujú podiel v rámci daného typu počasia a celkový počet nehôd v dobrom počasí je výrazne vyšší než v zlom.">?</span>
                        </h5>
                        <p class="text-muted">Tabuľka ukazuje podiel príčin v rámci daného počasia.</p>
                        <div class="table-container">
                            {table_good_html}
                        </div>
                    </div>
                </div>
            </div>
        </div>
        
    </div>

    </body>
    </html>
    """
    with open("doc.html", "w", encoding="utf-8") as f:
        f.write(html_content)


def analyze_weather_impact(df) -> None:
    """Analýza vplyvu počasia na príčiny nehôd a generovanie reportu."""
    # Príprava dát
    df["pocasie_typ"] = np.where(df["p18"] == 1, good, bad)
    df["pricina_nazov"] = df["p12"].map(CAUSE_MAP).fillna("Ostatné príčiny")

    stats = df.groupby(["pocasie_typ", "pricina_nazov"]).size().reset_index(name="count")
    weather_totals = df.groupby("pocasie_typ").size().reset_index(name="total_weather")
    stats = stats.merge(weather_totals, on="pocasie_typ")
    stats["percent"] = (stats["count"] / stats["total_weather"]) * 100

    # --- Vypočítané hodnoty pre text ---
    total_accidents = len(df)
    bad_weather_accidents = weather_totals[weather_totals["pocasie_typ"] == bad]["total_weather"].values[0]
    bad_weather_ratio = (bad_weather_accidents / total_accidents) * 100

    # Analýza "Neprispôsobenie rýchlosti stavu vozovky" (kód 204)
    target_cause_name = CAUSE_MAP[204]
    cause_204_mask = stats["pricina_nazov"] == target_cause_name

    pct_204_bad = stats[cause_204_mask & (stats["pocasie_typ"] == bad)]["percent"].values[0]
    pct_204_good = stats[cause_204_mask & (stats["pocasie_typ"] == good)]["percent"].values[0]

    risk_ratio = pct_204_bad / pct_204_good if pct_204_good > 0 else 0

    # Analýza "Nevěnování se řízení" (kód 508)
    cause_508_mask = stats["pricina_nazov"].str.contains("nevěnoval řízení", case=False)
    pct_508_good = (
        stats[cause_508_mask & (stats["pocasie_typ"] == good)]["percent"].values[0]
        if not stats[cause_508_mask & (stats["pocasie_typ"] == good)].empty
        else 0
    )
    pct_508_bad = (
        stats[cause_508_mask & (stats["pocasie_typ"] == bad)]["percent"].values[0]
        if not stats[cause_508_mask & (stats["pocasie_typ"] == bad)].empty
        else 0
    )

    # --- Generovanie textu ---
    text_report = f"""
    <p>Táto analýza vyhodnocuje vplyv meteorologických podmienok na nehodovosť v Českej republike na základe <b>{total_accidents}</b> záznamov. 
    Dáta sme rozdelili na nehody za ideálnych podmienok a nehody za zhoršeného počasia (dážď, hmla, námraza, sneženie).</p>
    
    <h5>Hlavné zistenia analýzy:</h5>
    <ul>
        <li>
            <strong>Rozsah problému:</strong> Za zhoršených poveternostných podmienok sa stalo <b>{bad_weather_accidents}</b> nehôd. 
            Hoci to predstavuje "len" <b>{bad_weather_ratio:.1f} %</b> z celkového počtu, štruktúra príčin sa v týchto momentoch dramaticky mení.
        </li>
        <li>
            <strong>Riziko v zlom počasí:</strong> Najkritickejším faktorom je neprispôsobenie rýchlosti stavu vozovky. 
            Kým v dobrom počasí je táto chyba zriedkavá ({pct_204_good:.1f} %), v zlom počasí tvorí až <b>{pct_204_bad:.1f} %</b> všetkých nehôd. 
            Riziko šmyku či straty kontroly je tak v zlom počasí až <b>{risk_ratio:.1f}-násobne vyššie</b>.
        </li>
        <li>
            <strong>Paradox dobrého počasia:</strong> Grafy odhaľujú zaujímavý psychologický fenomén. 
            Najčastejšou príčinou nehôd v dobrom počasí je nepozornosť ("nevěnování se řízení"), ktorá tvorí <b>{pct_508_good:.1f} %</b> prípadov.
            V zlom počasí podiel tejto príčiny klesá na <b>{pct_508_bad:.1f} %</b>.
        </li>
    </ul>

    <h5>Záver:</h5>
    <p>Z analýzy vyplýva jasný trend. Zhoršené počasie kladie nároky na techniku jazdy (zvládnutie šmyku), čo mnoho vodičov nezvláda a končí mimo vozovky. 
    Naopak, <strong>priaznivé počasie vyvoláva falošný pocit bezpečia</strong>. Vodiči majú v ideálnych podmienkach menšiu tendenciu udržiavať pozornosť, 
    čo vedie k tomu, že nepozornosť je v slnečných dňoch výrazne dominantnejšou príčinou tragédií než v daždi, kedy sú vodiči ostraženejší.</p>
    """

    # Pomocná funkcia na prípravu DataFrame pre graf a export
    def prepare_data(target_weather):
        target_stats = stats[stats["pocasie_typ"] == target_weather]
        top_5_causes = target_stats.sort_values("count", ascending=False).head(5)["pricina_nazov"].tolist()

        plot_df = stats[stats["pricina_nazov"].isin(top_5_causes)].copy()

        # Zoradenie
        plot_df["pricina_nazov"] = pd.Categorical(plot_df["pricina_nazov"], categories=top_5_causes, ordered=True)
        plot_df = plot_df.sort_values("pricina_nazov")

        return plot_df

    # --- ZHORŠENÉ POČASIE ---
    df_bad_plot = prepare_data(bad)
    plot_weather_impact_plotly(df_bad_plot, "Top 5 príčin: Zhoršené počasie (vs. Dobré)", "fig_1.html")
    table_bad = df_bad_plot.pivot(index="pricina_nazov", columns="pocasie_typ", values="count").fillna(0)

    # --- DOBRÉ POČASIE ---
    df_good_plot = prepare_data(good)
    plot_weather_impact_plotly(df_good_plot, "Top 5 príčin: Dobré počasie (vs. Zhoršené)", "fig_2.html")
    table_good = df_good_plot.pivot(index="pricina_nazov", columns="pocasie_typ", values="count").fillna(0)

    print_info(table_bad, table_good, total_accidents, bad_weather_accidents, risk_ratio)

    # Generovanie HTML
    generate_html_report(table_bad, table_good, text_report)


if __name__ == "__main__":
    analyze_weather_impact(load_data())
