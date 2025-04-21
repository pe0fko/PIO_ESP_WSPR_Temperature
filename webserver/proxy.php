<?php
header('Content-Type: application/json');

// Verkrijg het echte IP-adres, rekening houdend met Cloudflare
$ip = $_SERVER['HTTP_CF_CONNECTING_IP'] ?? $_SERVER['REMOTE_ADDR'];


// Check of het een IPv6-adres is
if (filter_var($ip, FILTER_VALIDATE_IP, FILTER_FLAG_IPV6)) {
    // Als het een IPv6-adres is, kun je hier aanvullende logica toepassen of het IPv6 gebruiken
    // bijvoorbeeld: $ip = ipv6_to_ipv4($ip); (indien gewenst)
//    echo "IPv6: $ip\n";
	$ip = ipv6_to_ipv4($ip);
} else {
    // Als het een IPv4-adres is
//    echo "IPv4: $ip\n";
}

// Voor debugging: toon het IP-adres
// echo "Client IP: " . $ip . "<br>";

// Het IP-adres is nu het echte IP van de gebruiker (via Cloudflare)

$ipinfo = json_decode(file_get_contents("https://ipinfo.io/{$ip}/json"), true);
list($lat, $lon) = explode(',', $ipinfo['loc'] ?? "52.0,5.0");



// ---- Stap 1: IP detectie en locatie ophalen via ipinfo.io ----
//$ip = $_SERVER['REMOTE_ADDR'];
//$ipinfo = json_decode(file_get_contents("https://ipinfo.io/{$ip}/json"), true);
//list($lat, $lon) = explode(',', $ipinfo['loc'] ?? "52.0,5.0");

// ---- Stap 2: QTH-berekening ----
function maidenhead($lat, $lon) {
    $lat += 90;
    $lon += 180;

    $A = ord('A');
    $qth = '';
    $qth .= chr($A + intval($lon / 20));
    $qth .= chr($A + intval($lat / 10));
    $qth .= strval(intval(($lon % 20) / 2));
    $qth .= strval(intval($lat % 10));
    $qth .= chr($A + intval((($lon % 2) * 60) / 5));
    $qth .= chr($A + intval((($lat - intval($lat)) * 60) / 2.5));
    return $qth;
}
$qth = maidenhead(floatval($lat), floatval($lon));

// ---- Stap 3: Landcode via OpenStreetMap (Nominatim) ----
$nominatim_url = "https://nominatim.openstreetmap.org/reverse?format=json&lat=$lat&lon=$lon&zoom=3&addressdetails=1";
$opts = ['http' => ['header' => "User-Agent: qth-proxy/1.0\r\n"]];
$context = stream_context_create($opts);
$nominatim_json = file_get_contents($nominatim_url, false, $context);
$nominatim_data = json_decode($nominatim_json, true);
$landcode = strtolower($nominatim_data['address']['country_code'] ?? 'xx');

// ---- Stap 4: Zon op / onder via sunrise-sunset.org ----
$sun_url = "https://api.sunrise-sunset.org/json?lat=$lat&lng=$lon&formatted=0";
$sun_data = json_decode(file_get_contents($sun_url), true);

function parse_utc_time($timeStr) {
    $time = strtotime($timeStr);
    $time += date('Z'); // server UTC → lokale tijdzone
    return [
        'uur' => intval(date('G', $time)),
        'min' => intval(date('i', $time))
    ];
}

$zon_op = parse_utc_time($sun_data['results']['sunrise']);
$zon_onder = parse_utc_time($sun_data['results']['sunset']);

// ---- Stap 5: Eigen configuratie ophalen ----
$configs = json_decode(file_get_contents("config.json"), true);
$config = $configs[$ip] ?? $configs[$landcode] ?? [];

// ---- Output JSON ----
echo json_encode([
    "ip" => $ip,
    "qth" => $qth,
    "land" => $landcode,
    "zon" => [
        "op" => $zon_op,
        "onder" => $zon_onder
    ],
    "config" => $config
]);
