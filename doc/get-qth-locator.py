#!/usr/bin/python3
#
import requests
import math

def get_ip_location():
    """Haalt de GPS-coördinaten op via ipinfo.io."""
    response = requests.get("https://ipinfo.io/json")
    data = response.json()
    print(f"JSON: {data}")
    if "loc" in data:
        lat, lon = map(float, data["loc"].split(","))
        return lat, lon
    else:
        raise ValueError("Kon locatie niet ophalen van ipinfo.io.")

def latlon_to_maidenhead(lat, lon):
    """Converteert breedte- en lengtegraad naar een Maidenhead QTH-locator."""
    A = ord('A')

    # Basis gridletters
    lon += 180
    lat += 90

    field_lon = int(lon / 20)
    field_lat = int(lat / 10)
    square_lon = int((lon % 20) / 2)
    square_lat = int((lat % 10) )
    subsquare_lon = int(((lon % 2) / 2) * 24)
    subsquare_lat = int(((lat % 1) ) * 24)

    maidenhead = (
        chr(A + field_lon) +
        chr(A + field_lat) +
        str(square_lon) +
        str(square_lat) +
        chr(A + subsquare_lon) +
        chr(A + subsquare_lat)
    )

    return maidenhead

if __name__ == "__main__":
    lat, lon = get_ip_location()
    maidenhead_locator = latlon_to_maidenhead(lat, lon)
    print(f"Jouw QTH-locator is: {maidenhead_locator}")
