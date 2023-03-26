//const char* ssid = "Torre C Labs";
//const char* password = "1q2w3e4r5t";
const char* ssid = "MEGACABLE-2.4G-9FA7";
const char* password = "57q2dwMHN7";

WiFiServer server(80);

void conectarWiFi() {
  Serial.print("Conectando a  ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi conectado.");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());

  server.begin();
}