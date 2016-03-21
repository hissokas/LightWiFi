#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "YourSSID";
const char* password = "YourPassword";

ESP8266WebServer server(80);

bool is_authentified(){
  Serial.println("Enter is_authentified");
  if (server.hasHeader("Cookie")){   
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentification Successful");
      return true;
    }
  }
  Serial.println("Authentification Failed");
  return false;	
}

//login page, also called for disconnect/Pagina del Login Tambien es usada para desconectar
void handleLogin(){
  String msg;
  if (server.hasHeader("Cookie")){   
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")){
    Serial.println("Disconnection");
    String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID=0\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")){
    if (server.arg("USERNAME") == "YourUSER" &&  server.arg("PASSWORD") == "YourPASSWORD" ){ //HERE YOU CHANGE TH USER /AQUI CAMBIAS DE USUARIO
      String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID=1\r\nLocation: /\r\nCache-Control: no-cache\r\n\r\n";
      server.sendContent(header);
      Serial.println("Welcome");
      return;
    }
  msg = "Wrong username/password! try again.";
  Serial.println("Log in Failed");
  }
  String content = "<html><body><form action='/login' method='POST'>Para accesar ingresa tu usuario y contrase&ntilde;a<br>";
  content += "Usuario:<input type='text' name='USERNAME' placeholder='user name'><br>";
  content += "Contrase&ntilde;a:<input type='password' name='PASSWORD' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
  content += "</body></html>";
  server.send(200, "text/html", content);
}

//root page can be accessed only if authentification is ok/Pagina principal solo puede accesar si se pasa la autentificacion 
void handleRoot(){
  Serial.println("Enter handleRoot");
  String header;
  if (!is_authentified()){
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  if (server.hasHeader("User-Agent")){
  String prue = "<html><body><H2>Control de Cuarto</H2><form action='/boton' method='POST'>Presiona para Apagar/prender luz<br>";
  prue += "<input type='submit' name='luz' value='Luz'></form>";
  prue += "Cerrar sesion <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
  server.send(300, "text/html", prue);  
  
  }
}

//no need authentification
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");
  pinMode(13,OUTPUT);
  while (WiFi.status() != WL_CONNECTED) {     // Wait for connection
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/boton", preBoton);
  

  server.onNotFound(handleNotFound);  //here the list of headers to be recorded
  
  const char * headerkeys[] = {"User-Agent","Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize );
  if (!MDNS.begin("esp8266")) {
    Serial.println("Error setting up MDNS responder!");
     delay(1000);
    
  }
  Serial.println("mDNS responder started");
  server.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.println("HTTP server started");
}

void preBoton()
  { digitalWrite(13,!digitalRead(13)); 
    handleRoot();
    }


void loop(void){
  server.handleClient();
}
