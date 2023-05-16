#!/usr/bin/env python
# coding: utf-8

# Configuracion inicial
import pandas  # libreria para procesamiento de datos muy usada en la industria
import matplotlib.pyplot as plt  # librería para graficas de python
import json
import seaborn  # Extiende las funcionalidades de matplotlib, muy usado en data visualization
import numpy  # extiende las librerias de matemática de python

# Con esto hacemos los gráficos más grandes, adecuados para el proyector.
seaborn.set_context(context='talk', font_scale=1.2)

TOTAL_TIME = 200


class DfWrapper:
    def __init__(self, file_path):
        self.file_path = file_path
        self.data = None

        self.set_data_from_json()

    @staticmethod
    def load_json_file(file_path):
        with open(file_path) as json_file:
            data = json.load(json_file)
        return data

    def set_data_from_json(self):
        json_data = self.load_json_file(self.file_path).values()
        self.data = pandas.DataFrame.from_dict(json_data)

    def get_val_from_col(self, col, key):
        result = []
        for row in self.data[col]:
            for field in row:
                if field['name'] == key:
                    result.append(field['value'])
        return result


def get_delays(data):
    return data.get_val_from_col("scalars", "avgDelay")


def get_gen_intervals(data):
    interval_strings = data.get_val_from_col(
        "parameters", "generationInterval")
    return [float(interval.split("(")[-1][:-1]) for interval in interval_strings]


def draw_delay(data, index, rindex):
    gen_intervals, delays = get_gen_intervals(data), get_delays(data)
    print(gen_intervals)
    plt.plot(gen_intervals, delays, linewidth=0.5 + rindex*2,  zorder=index,
             label=f"{data.file_path.split('.json')[0]}")


# Calcular la carga útil
def draw_carga(data, index, rindex):
    paquetes_emisor = data.get_val_from_col("scalars", "sentPackets")
    paquetes_receptor = data.get_val_from_col("scalars", "receivedPackets")
    carga_ofrecida = [paquetes/TOTAL_TIME for paquetes in paquetes_emisor]
    carga_util = [paquetes/TOTAL_TIME for paquetes in paquetes_receptor]
    plt.plot(carga_ofrecida, carga_util, linewidth=0.5 + rindex*2, zorder=index,
             label=f"{data.file_path.split('.json')[0]}")


def draw_with_func(sources, draw_func):
    source_length = len(sources)
    for index, source in enumerate(sources):
        data = DfWrapper(source)
        draw_func(data, index, source_length - index)
    plt.legend(loc='best')
    plt.show()


def draw_delays(sources):
    plt.xlabel("generationInterval")
    plt.ylabel("Average Delay")
    draw_with_func(sources, draw_delay)


def draw_cargas(sources):
    plt.xlabel("Carga Ofrecida (pps)")
    plt.ylabel("Carga Util (pps)")
    draw_with_func(sources, draw_carga)


def main():
    plt.style.use("ggplot")
    sources = [
        "tc1p1.json",
        "tc2p1.json",
        "tc1p2.json",
        "tc2p2.json",
        "tc3p2.json",
    ]
    draw_delays(sources)
    draw_cargas(sources)


if __name__ == '__main__':
    main()
