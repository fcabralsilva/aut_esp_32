String VERSAO = "V0711 - 26/05/2019";
//---------------------------------------
//    INCLUINDO BIBLIOTECAS
//---------------------------------------
#include <BluetoothSerial.h>
//#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <Alarme.h>
#include <ArduinoJson.h>
//#include <DNSServer.h>
#include <DHT.h>
#include <EEPROM.h>
#include <FS.h>
//#include <HTTPClient.h>
#include <NTPClient.h>
#include <RCSwitch.h>
#include <SPIFFS.h>
//#include <Update.h>
//#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <Wire.h>
#include <WiFi.h>
#include <WiFiUDP.h>
//#include <WiFiUdp.h>
#include <WebServer.h>
#include <WiFiManager.h>

//---------------------------------------
//    DEFINIÇÕES DE VARIAVEIS FIXAS
//---------------------------------------
#define BUZZER                5
#define PIN_MQ2               34
#define DHTPIN                19
#define DHTTYPE               DHT11
#define VRL_VALOR             5       //resistência de carga
#define RO_FATOR_AR_LIMPO     9.83    //resistência do sensor em ar limpo 9.83 de acordo com o datasheet
#define ITERACOES_CALIBRACAO  25      //numero de leituras para calibracao
#define ITERACOES_LEITURA     5       //numero de leituras para analise
#define GAS_LPG               0
#define GAS_CO                1
#define SMOKE                 2
#define RF_RECEIVER           35
#define PCF8574_ADDR		(0x20)
//---------------------------------------
#define LED_AZUL              2
#define LED_VERDE             15
#define LED_VERMELHO          4
long milis = 0;        // último momento que o LED foi atualizado
long interval = 250;           // tempo de transição entre estados (milisegundos)
//---------------------------------------
//    INSTANCIANDO STRUCT'S
//---------------------------------------
struct botao1 {
  int entrada = 32, rele = 33;
  boolean estado = 0, estado_atual = 0  , estado_antes = 0;
  int contador = 0;
  const char* modelo = "interruptor";
  const char* nomeInter = "Com1";
  const char* tipo = "0";
} botao1;
struct botao2 {
  int entrada = 25, rele = 26;
  boolean estado = 0, estado_atual = 0  , estado_antes = 0;
  int contador = 0;
  const char* modelo = "interruptor";
  const char* tipo = "0";
  const char* nomeInter = "Com2";
} botao2;
struct botao3 {
  int entrada = 14, rele = 27;
  boolean estado = 0, estado_atual = 0  , estado_antes = 0;
  int contador = 0;
  const char* tipo = "0";
  const char* modelo = "interruptor";
  const char* nomeInter = "Com3";
} botao3;
struct botao4 {
  int entrada = 12, rele = 13;
  boolean estado = 0, estado_atual = 0  , estado_antes = 0;
  int contador = 0;
  const char* tipo = "0";
  const char* modelo = "interruptor";
  const char* nomeInter = "Com4";
} botao4;
struct botao5 {
  int entrada = 16, rele = 18;
  boolean estado = 0, estado_atual = 0  , estado_antes = 0;
  int contador = 0;
  const char* tipo = "0";
  const char* modelo = "interruptor";
  const char* nomeInter = "Com5";
} botao5;

//---------------------------------------
//    INICIANDO VARIAVEIS
//---------------------------------------
String ipLocalString, buff, URL, linha, GLP, FUMACA, retorno, serv, logtxt = "sim", hora_ntp, hora_rtc,  LIMITE_MQ2, buf, IP_FIXO, GATEWAY, MASCARA_IP;
const char *json;
const char *ssid, *password, *servidor, *conslog, *nivelLog = "4", *verao, *s_senha_alarme = "123456";
const int PIN_AP = 3, i_sensor_alarme = 17, i_sirene_alarme = 18;
int portaServidor = 80, contarParaGravar2 = 0 ;
int contarParaGravar1 = 0, nContar = 0, cont_ip_banco = 0, nivel_log = 4, estado_atual = 0, estado_antes = 0, freq = 2000, channel = 0, resolution = 8, n = 0, sensorMq2 = 0, contadorPorta = 0, T_WIFI = 50, REINICIO_CENTRAL, MEM_EEPROM_MQ2 = 20;
short paramTempo = 60;
unsigned long time1sec, time3, time3Param = 100000, timeDht, timeMq2 , tempo = 0, timeDhtParam = 300000, timeMq2Param = 30000;
IPAddress ipHost;
WiFiUDP ntpUDP;
int16_t utc = -3; //UTC -3:00 Brazil
uint32_t currentMillis = 0;
uint32_t previousMillis = 0;
NTPClient timeClient(ntpUDP, "a.st1.ntp.br", -3 * 3600, 60000);
float umidade = 0, temperatura = 0;
float LPGCurve[3]  =  {2.3, 0.20, -0.47}; //curva LPG aproximada baseada na sensibilidade descrita no datasheet {x,y,deslocamento} baseada em dois pontos
//p1: (log200, log1.6), p2: (log10000, log0.26)
//inclinacao = (Y2-Y1)/(X2-X1)
//vetor={x, y, inclinacao}
float COCurve[3]  =  {2.3, 0.72, -0.34};  //curva CO aproximada baseada na sensibilidade descrita no datasheet {x,y,deslocamento} baseada em dois pontos
//p1: (log200, 0.72), p2(log10000, 0.15)
//inclinacao = (Y2-Y1)/(X2-X1)
//vetor={x, y, inclinacao}
float SmokeCurve[3] = {2.3, 0.53, -0.44}; //curva LPG aproximada baseada na sensibilidade descrita no datasheet {x,y,deslocamento} baseada em dois pontos
//p1: (log200, 0.53), p2: (log10000, -0.22)
//inclinacao = (Y2-Y1)/(X2-X1)
//vetor={x, y, inclinacao}
float Ro = 10;
// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;
boolean b_status_alarme = 0;
//---------------------------------------
//    INSTANCIANDO CLASSES
//---------------------------------------
DHT dht(DHTPIN, DHTTYPE);
WiFiServer server(80);
Alarme alarme;
RCSwitch mySwitch = RCSwitch();
BluetoothSerial ESP_BLUT;
//---------------------------------------

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);
  EEPROM.begin(64);
  dht.begin();
  mySwitch.enableReceive(RF_RECEIVER);
  delay(2000);
  pinMode(PIN_AP, INPUT_PULLUP);

  pinMode(botao1.rele, OUTPUT);
  pinMode(botao1.entrada, INPUT_PULLUP);
  digitalWrite(botao1.rele, LOW);

  pinMode(botao2.rele, OUTPUT);
  pinMode(botao2.entrada, INPUT_PULLUP);
  digitalWrite(botao2.rele, LOW);

  pinMode(botao3.rele, OUTPUT);
  pinMode(botao3.entrada, INPUT_PULLUP);
  digitalWrite(botao3.rele, LOW);

  pinMode(botao4.rele, OUTPUT);
  pinMode(botao4.entrada, INPUT_PULLUP);
  digitalWrite(botao4.rele, LOW);
  
  pinMode(botao5.rele, OUTPUT);
  pinMode(botao5.entrada, INPUT_PULLUP);
  digitalWrite(botao5.rele, LOW);

  pinMode(0, INPUT);

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  pinMode(LED_AZUL, OUTPUT);
  digitalWrite(LED_AZUL, LOW);
  pinMode(LED_VERDE, OUTPUT);
  digitalWrite(LED_VERDE, LOW);
  pinMode(LED_VERMELHO, OUTPUT);
  digitalWrite(LED_VERMELHO, LOW);
  alarme.sensores(i_sensor_alarme);
  alarme.sirene(i_sirene_alarme);
  //---------------------------------------
  //    LOG
  //---------------------------------------
  openFS();
  criarArquivo("/param.txt");
  criarArquivo("/log.txt");
  //---------------------------------------
  //    CONECTANDO A REDE WIFI
  //---------------------------------------
  Serial.println("----------------------------------");
  Serial.print(" Conectado a rede WIFI  ");
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
//  IPAddress _ip = IPAddress(192, 168, 0, 21);
//  IPAddress _gw = IPAddress(192, 168, 0, 1);
//  IPAddress _sn = IPAddress(255, 255, 255, 0);
//  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
  if (!wifiManager.autoConnect("WIFI_AUT", "12345678")) {
    Serial.println("Falha ao conectar e atingir o tempo limite");
    ESP.restart();
    delay(1000);
  }
  server.begin();
  ipHost = WiFi.localIP();
  ipLocalString = String(ipHost[0]) + "." + String(ipHost[1]) + "." + String(ipHost[2]) + "." + String(ipHost[3]);
  Serial.println("----------------------------------");
  Serial.println(" *Configurações da Central:");
  Serial.println(" IP da Central: " + ipLocalString);
  //---------------------------------------
  // PARAMETROS DA MEMORIA EEPROM
  //---------------------------------------
  LIMITE_MQ2 = byte(EEPROM.read(MEM_EEPROM_MQ2));
  Serial.println(" Valor sensor de Gás: " + String(LIMITE_MQ2));
  //---------------------------------------
  //PRIMEIRA LEITURA DO SENSORES
  //---------------------------------------
  //DHT11
  umidade = dht.readHumidity() * 1;
  temperatura = dht.readTemperature() * 1;
  //MQ2
  sensorMq2 = analogRead(PIN_MQ2);
  GLP = String(getQuantidadeGasMQ(leitura_MQ2(PIN_MQ2) / Ro, GAS_LPG) );
  FUMACA = String(getQuantidadeGasMQ(leitura_MQ2(PIN_MQ2) / Ro, SMOKE));
  //---------------------------------------

  //---------------------------------------
  //CALIBRAR LEITURA DO SENSOR DE GAS E FUMACA
  //---------------------------------------
  calibrarSensor();
  //---------------------------------------
  EEPROM.end();
  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  retorno = "SERVIDOR_CONECT";
  timeClient.begin();
  timeClient.forceUpdate();
  //timeClient.setTimeOffset(-6200);
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(5, channel);
  gravarArquivo(" \n\n ******************************* \n *** INICIANDO CENTRAL *** \n *******************************\n " + VERSAO, "log.txt");
  //closeFS();
}

void loop() {
  ArduinoOTA.handle();
  WiFiManager wifiManager;
  WiFiClient client = server.available();
  pisca_led(LED_VERDE, true);
  relogio();
  while (cont_ip_banco < 1)
  {
    openFS();
    //Gravando no log o reinicio da central
    StaticJsonDocument<700> doc;
    json = lerArquivoParam().c_str();
    DeserializationError error = deserializeJson(doc, json);
    if (error)
    {
      gravaLog(" " + hora_ntp + " - ERRO 0101 - Arquivo json: ", logtxt, 1);
      Serial.println(error.c_str());
      cont_ip_banco++;
      return;
    }
    JsonObject root = doc.as<JsonObject>();
    gravaLog(" " + hora_ntp + " - Configurações da Central ", logtxt, 2);
    servidor      = root["servidor"];
    serv = String(servidor);
    gravaLog(" " + hora_ntp + "   BD:   " + String(servidor), logtxt, 2);

    botao1.nomeInter  = root["int_1"];
    botao1.tipo     = root["tipo_1"];
    botao1.modelo     = root["sinal_1"];
    gravaLog(" " + hora_ntp + "   Int 1: " + String(botao1.nomeInter) + " / " + String(botao1.tipo) + " / " + String(botao1.modelo), logtxt, 2);

    botao2.nomeInter  = root["int_2"];
    botao2.tipo     = root["tipo_2"];
    botao2.modelo     = root["sinal_2"];
    gravaLog(" " + hora_ntp + "   Int 2: " + String(botao2.nomeInter) + " / " + String(botao2.tipo) + " / " + String(botao2.modelo), logtxt, 2);

    botao3.nomeInter  = root["int_3"];
    botao3.tipo     = root["tipo_3"];
    botao3.modelo     = root["sinal_3"];
    gravaLog(" " + hora_ntp + "   Int 3: " + String(botao3.nomeInter) + " / " + String(botao3.tipo) + " / " + String(botao3.modelo), logtxt, 2);

    botao4.nomeInter  = root["int_4"];
    botao4.tipo     = root["tipo_4"];
    botao4.modelo     = root["sinal_4"];
    gravaLog(" " + hora_ntp + "   Int 4: " + String(botao4.nomeInter) + " / " + String(botao4.tipo) + " / " + String(botao4.modelo), logtxt, 2);

    botao5.nomeInter  = root["int_5"];
    botao5.tipo     = root["tipo_5"];
    botao5.modelo     = root["sinal_5"];
    gravaLog(" " + hora_ntp + "   Int 5: " + String(botao5.nomeInter) + " / " + String(botao5.tipo) + " / " + String(botao5.modelo), logtxt, 2);

    conslog   = root["log"];
    logtxt = String(conslog);
    nivelLog = root["nivel"];
    verao = root["verao"];
    s_senha_alarme = root["senha_alarme"];
    gravaLog(" " + hora_ntp + "   Log ?: " + String(conslog) + " N: " + String(nivelLog), logtxt, 1);
    //gravaLog(" " + hora_ntp + "   Senha Alarme : " + String(s_senha_alarme), logtxt, 1);
    Serial.println("");
    cont_ip_banco++;
  }
  /*
	VERIFICA SE TEM COMANDOS RADIO FREQUENCIA ENVIADO
  */
  if (mySwitch.available())
  {
    output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(), mySwitch.getReceivedProtocol());
    mySwitch.resetAvailable();
  }
  /*
	VERIFICA SE FOI ENVIADO COMANDOS NO BLUETOOAPH
  */
    if (ESP_BLUT.available())
  {
    char incoming = ESP_BLUT.read(); //Read what we recevive
    Serial.print(" RF:"); Serial.println(incoming);
	gravaLog(" " + hora_ntp + " - RF:  "+incoming, logtxt, 1);
  }
  
  /*
  	SE FOR PRESSIONADO BOTÃO, TODAS AS CONFIGURAÇÕES DA CENTRAL SERÃO DELETADAS(WIFI, PARAMETROS, ETC). VARIAVEL DO BOTÃO É PIN_AP.
  */
  if ( digitalRead(PIN_AP) == LOW )
  {

    gravaLog(" " + hora_ntp + " - Modo AP apagando configurações WIFI ", logtxt, 1);
    /*
    	Apagando dados de conexão WIFI da central
    */
    esp_wifi_restore();
    Serial.println("\n Apagando configurações WIFI..."); //tenta abrir o portal
    delay(2000);
    if (!wifiManager.startConfigPortal(" WIFI_AUT", "12345678") )
    {
      gravaLog(" " + hora_ntp + " - ERRO 0102 - Falha ao conectar no WIFI modo AP (Access Poin)", logtxt, 1);
      delay(2000);
      ESP.restart();
      delay(1000);
    }
    Serial.println(" Central em modo de configuração do WIFI...");

  }
  //---------------------------------------
  //    ENTRADA E SAIDA 1
  //---------------------------------------
  String s_tipo_1 = String(botao1.tipo);
  String s_modelo_1 = String(botao1.modelo);
  if (s_modelo_1 == "pulso")
  {
    if (digitalRead(botao1.entrada) == s_tipo_1.toInt())
    {
      if (nContar == 0)Serial.println("\n"); Serial.println("\n E1 Pulso");
      while ((digitalRead(botao1.entrada) == s_tipo_1.toInt()) && (nContar <= 300) )
      {
        if (millis() >= tempo + paramTempo)
        {
          botao1.contador++;
          nContar++;
          Serial.print(botao1.contador, DEC);
          tempo = millis();
        }
      }
    }
  } else if (s_modelo_1 == "interruptor")
  {

    botao1.estado_atual = digitalRead(botao1.entrada);
    if (botao1.estado_atual != botao1.estado_antes )
    {
      if (nContar == 0)Serial.println(" E1 Inter");
      botao1.estado_antes = botao1.estado_atual;
      botao1.contador = 3;
      //Serial.print(botao1.contador, DEC);
    }
  } else if (s_modelo_1 == "pir")
  {
    botao1.estado_atual = digitalRead(botao1.entrada);
    if(botao1.estado_atual == true)
    {
      Serial.println(" Movimento Detectado...");
      }
  }
  if ((botao1.contador >= 2) && (botao1.contador <= 9))
  {
    if (nContar >= 100)
    {

      if (n == 0)
      {
        for (int i = 0; i <= 0 ; i++ )
        {
          String ERRO_ENTRADA = hora_rtc + " - ERRO 0107 - Interruptor 1 (Porta IN: " + botao1.rele + " Porta OUT: " + botao1.entrada + ") com erro de execução, deve usar a pagina para reiniciar";
          //Gravando log de erro na central.
          if ((nivel_log >= 1) || (logtxt == "sim")) gravarArquivo( ERRO_ENTRADA, "log.txt");
          n = 1;
        }
      }
    } else
    {
      String ERRO_ENTRADA = "0";
      nContar = 0;
      if (botao1.estado == false) {
        Serial.println(" Ligando Porta (rele 1): " + String(botao1.rele));
        botao1.estado = true;
        botao1.contador = 0;
        acionaPorta(botao1.rele, "", "liga");
      } else {
        Serial.println(" Desligar Porta (rele 1): " + String(botao1.rele));
        acionaPorta(botao1.rele, "", "desl");
        botao1.estado = false;
        botao1.contador = 0;
      }
    }
  }
  //---------------------------------------

  //---------------------------------------
  //    ENTRADA E SAIDA 2
  //---------------------------------------
  String s_tipo_2 = String(botao2.tipo);
  String s_modelo_2 = String(botao2.modelo);
  if (s_modelo_2 == "pulso")
  {
    if (digitalRead(botao2.entrada) == s_tipo_2.toInt())
    {
      if (nContar == 0)Serial.println(" E2 Pulso");
      while ((digitalRead(botao2.entrada) == s_tipo_2.toInt()) && (nContar <= 300) )
      {
        if (millis() >= tempo + paramTempo)
        {
          botao2.contador++;
          nContar++;
          Serial.print(botao2.contador, DEC);
          tempo = millis();
        }
      }
    }
  } else if (s_modelo_2 == "interruptor")
  {
    botao2.estado_atual = digitalRead(botao2.entrada);
    if (botao2.estado_atual != botao2.estado_antes )
    {
      if (nContar == 0)Serial.println(" E2 Inter ");
      botao2.estado_antes = botao2.estado_atual;
      botao2.contador = 3;
      //Serial.print(botao2.contador, DEC);
    }
  }
  if ((botao2.contador >= 2) && (botao2.contador <= 9))
  {
    if (nContar >= 100)
    {
      for (int i = 0; i <= 0 ; i++ )
      {
        String ERRO_ENTRADA = " ERRO 0107 - Botão 2 com erro de execução, reiniciar central";
        //Gravando log de erro na central.
        if ((nivel_log >= 1) || (logtxt == "sim")) gravarArquivo( hora_rtc + " - ERRO 0107 - Botão 2 com erro de execução, reiniciar central", "log.txt");
      }
    } else
    {
      String ERRO_ENTRADA = "0";
      nContar = 0;
      if (botao2.estado == false) {
        Serial.println(" Ligando Porta (rele 2): " + String(botao2.rele));
        botao2.estado = true;
        botao2.contador = 0;
        acionaPorta(botao2.rele, "", "liga");
      } else {
        Serial.println(" Desligar Porta (rele 2): " + String(botao2.rele));
        acionaPorta(botao2.rele, "", "desl");
        botao2.estado = false;
        botao2.contador = 0;
      }
    }
  }
  //---------------------------------------

  //---------------------------------------
  //    ENTRADA E SAIDA 3
  //---------------------------------------
  String s_tipo_3 = String(botao3.tipo);
  String s_modelo_3 = String(botao3.modelo);
  if (s_modelo_3 == "pulso")
  {
    if (digitalRead(botao3.entrada) == s_tipo_3.toInt())
    {
      if (nContar == 0)Serial.println(" E3 Pulso");
      while ((digitalRead(botao3.entrada) == s_tipo_3.toInt()) && (nContar <= 300) )
      {
        if (millis() >= tempo + paramTempo)
        {
          botao3.contador++;
          nContar++;
          Serial.print(botao3.contador, DEC);
          tempo = millis();
        }
      }
    }
  } else if (s_modelo_3 == "interruptor")
  {
    botao3.estado_atual = digitalRead(botao3.entrada);
    if (botao3.estado_atual != botao3.estado_antes )
    {
      if (nContar == 0)Serial.print(" E3 Inter");
      botao3.estado_antes = botao3.estado_atual;
      botao3.contador = 3;
      //Serial.print(botao3.contador, DEC);
    }
  }
  if ((botao3.contador >= 2) && (botao3.contador <= 9))
  {
    if (nContar >= 100)
    {
      for (int i = 0; i <= 0 ; i++ )
      {
        String ERRO_ENTRADA = " ERRO 0107 - Botão 3 com erro de execução, reiniciar central";
        //Gravando log de erro na central.
        if ((nivel_log >= 1) || (logtxt == "sim")) gravarArquivo( hora_rtc + " - ERRO 0107 - Botão 3 com erro de execução, reiniciar central", "log.txt");
      }
    } else
    {
      String ERRO_ENTRADA = "0";
      nContar = 0;
      if (botao3.estado == false) {
        Serial.println(" Ligando Porta (rele 3): " + String(botao3.rele));
        botao3.estado = true;
        botao3.contador = 0;
        acionaPorta(botao3.rele, "", "liga");
      } else {
        Serial.println(" Desligar Porta (rele 3): " + String(botao3.rele));
        acionaPorta(botao3.rele, "", "desl");
        botao3.estado = false;
        botao3.contador = 0;
      }
    }
  }
  //---------------------------------------
  //---------------------------------------
  //    ENTRADA E SAIDA 4
  //---------------------------------------
  String s_tipo_4 = String(botao4.tipo);
  String s_modelo_4 = String(botao4.modelo);
  if (s_modelo_4 == "pulso")
  {
    if (digitalRead(botao4.entrada) == s_tipo_4.toInt())
    {
      if (nContar == 0)Serial.println("\n"); Serial.println("\n E4 Pulso");
      while ((digitalRead(botao4.entrada) == s_tipo_4.toInt()) && (nContar <= 300) )
      {
        if (millis() >= tempo + paramTempo)
        {
          botao4.contador++;
          nContar++;
          Serial.print(botao4.contador, DEC);
          tempo = millis();
        }
      }
    }
  } else if (s_modelo_4 == "interruptor")
  {

    botao4.estado_atual = digitalRead(botao4.entrada);
    if (botao4.estado_atual != botao4.estado_antes )
    {
      if (nContar == 0)Serial.println(" E4 Inter");
      botao4.estado_antes = botao4.estado_atual;
      botao4.contador = 3;
      //Serial.print(botao4.contador, DEC);
    }
  }
  if ((botao4.contador >= 2) && (botao4.contador <= 9))
  {
    if (nContar >= 100)
    {
      for (int i = 0; i <= 0 ; i++ )
      {
        String ERRO_ENTRADA = " ERRO 0107 - Botão 4 com erro de execução, reiniciar central";
        if ((nivel_log >= 1) || (logtxt == "sim")) gravarArquivo( hora_rtc + " - ERRO 0107 - Botão 4 com erro de execução, reiniciar central", "log.txt");
      }
    } else
    {
      String ERRO_ENTRADA = "0";
      nContar = 0;
      if (botao4.estado == false) {
        Serial.println("\n Ligando Porta (rele 4): " + String(botao4.rele));
        botao4.estado = true;
        botao4.contador = 0;
        acionaPorta(botao4.rele, "", "liga");
      } else {
        Serial.println("\n Desligar Porta (rele 4): " + String(botao4.rele));
        acionaPorta(botao4.rele, "", "desl");
        botao4.estado = false;
        botao4.contador = 0;
      }
    }
  }
  //---------------------------------------
  //    ENTRADA E SAIDA 5
  //---------------------------------------
  String s_tipo_5 = String(botao5.tipo);
  String s_modelo_5 = String(botao5.modelo);
  if (s_modelo_5 == "pulso")
  {
    if (digitalRead(botao5.entrada) == s_tipo_5.toInt())
    {
      if (nContar == 0)Serial.println("\n"); Serial.println("\n E5 Pulso");
      while ((digitalRead(botao5.entrada) == s_tipo_5.toInt()) && (nContar <= 300) )
      {
        if (millis() >= tempo + paramTempo)
        {
          botao4.contador++;
          nContar++;
          Serial.print(botao5.contador, DEC);
          tempo = millis();
        }
      }
    }
  } else if (s_modelo_5 == "interruptor")
  {

    botao5.estado_atual = digitalRead(botao5.entrada);
    if (botao5.estado_atual != botao5.estado_antes )
    {
      if (nContar == 0)Serial.println(" E5 Interr");
      botao5.estado_antes = botao5.estado_atual;
      botao5.contador = 3;
      //Serial.print(botao4.contador, DEC);
    }
  }
  if ((botao5.contador >= 2) && (botao5.contador <= 9))
  {
    if (nContar >= 100)
    {
      for (int i = 0; i <= 0 ; i++ )
      {
        String ERRO_ENTRADA = " ERRO 0107 - Botão 5 com erro de execução, reiniciar central";
        if ((nivel_log >= 1) || (logtxt == "sim")) gravarArquivo( hora_rtc + " - ERRO 0107 - Botão 5 com erro de execução, reiniciar central", "log.txt");
      }
    } else
    {
      String ERRO_ENTRADA = "0";
      nContar = 0;
      if (botao5.estado == false) {
        Serial.println("\n Ligando Porta (rele 5): " + String(botao5.rele));
        botao5.estado = true;
        botao5.contador = 0;
        acionaPorta(botao5.rele, "", "liga");
      } else {
        Serial.println("\n Desligar Porta (rele 5): " + String(botao4.rele));
        acionaPorta(botao5.rele, "", "desl");
        botao5.estado = false;
        botao5.contador = 0;
      }
    }
  }
  //---------------------------------------
  //    LIGAR E DESLIGAR TODOS RELES
  //---------------------------------------
  if (((botao1.contador >= 30) && (botao1.contador <= 50))
      || ((botao2.contador >= 30) && (botao2.contador <= 50))
      || ((botao3.contador >= 30) && (botao3.contador <= 50))
      || ((botao4.contador >= 30) && (botao4.contador <= 50)) 
      || ((botao5.contador >= 30) && (botao5.contador <= 50)) )
  {
    gravaLog(" " + hora_ntp + "\n - DESL. RELES", logtxt, 2);
    acionaPorta(botao1.rele, "", "desl");
    botao1.estado = false;
    acionaPorta(botao2.rele, "", "desl");
    botao2.estado = false;
    acionaPorta(botao3.rele, "", "desl");
    botao3.estado = false;
    acionaPorta(botao4.rele, "", "desl");
    botao4.estado = false;
	  acionaPorta(botao5.rele, "", "desl");
    botao5.estado = false;
    botao5.contador = 0;
	  botao4.contador = 0;
    botao3.contador = 0;
    botao2.contador = 0;
    botao1.contador = 0;
  }
  /*
    LEITURA DA REQUISIÇÃO DE CHAMADAS GET
  */
  if (client)
  {
    URL = "";
    URL = client.readStringUntil('\r');
  } else {
    URL = "vazio";
  }
  if (URL != "vazio")
  {
    /*
    	EXEMPLO NA CHAMADA WEB DESLIGAR LAMPADA - CHAMADA HTTP EX: HTTP://IP_HOST/?porta=20&acao=desligar&central=IP_HOST
    */
    String stringUrl = URL;
    gravaLog(" " + hora_ntp + " - " + String(URL), logtxt, 4);
    URL = "";
    String requisicao = stringUrl.substring(6, 11);
    if (requisicao == "porta") {
      String numero 	= stringUrl.substring(12, 14);
      String acao 		= stringUrl.substring(20, 24);
      String central 	= stringUrl.substring(33, 40);
      int numeroInt 	= numero.toInt();
      nContar = 0;
      n = 0;
      acionaPorta(numeroInt, requisicao, acao);
      if (numeroInt == botao1.rele) {
        if (acao == "liga") {
          botao1.estado = true;
        } else {
          botao1.estado = false;
        }
      } else if (numeroInt == botao2.rele) {
        if (acao == "liga") {
          botao2.estado = true;
        } else {
          botao2.estado = false;
        }
      } else if (numeroInt == botao3.rele) {
        if (acao == "liga") {
          botao3.estado = true;
        } else {
          botao3.estado = false;
        }
      } else if (numeroInt == botao4.rele) {
        if (acao == "liga") {
          botao4.estado = true;
        } else {
          botao4.estado = false;
        }
      }else if (numeroInt == botao5.rele) {
        if (acao == "liga") {
          botao5.estado = true;
        } else {
          botao5.estado = false;
        }
      }
      /*
        String buff;
        buff += "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://"+String(servidor[0])+"."+String(servidor[1])+"."+String(servidor[2])+"."+String(servidor[3])+"\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n";
        client.print(buff);
        buff = "";
      */
    }
    /*
    			REINCIAR CENTRAL POR COMANDA HTTP - CHAMADA HTTP EX: HTTP://IP_HOST/?00000
    */
    if (requisicao == "00000")
    {
      gravaLog(" " + hora_ntp + " - Central reiniciada p/ WEB", logtxt, 1);
      delay(1000);
      ESP.restart();
    }
    /*
    			CALIBRAR SENSOR MQ2 - CHAMADA HTTP EX: HTTP://IP_HOST/?00010
    */
    if (requisicao == "00010")
    {
      gravaLog(" " + hora_ntp + " - Recalibrando sensor MQ-X", logtxt, 2);
      calibrarSensor();
    }
    /*
    			GRAVAR VALOR DE LEITURA DO SENSOR DE GAS NA EEPROM - CHAMADA HTTP EX: HTTP://IP_HOST/?00011
    */
    String codidoExec = stringUrl.substring(10, 15);
    int valorMQ_Novo = stringUrl.substring(22, 24).toInt();
    if (codidoExec == "00011")
    {
      gravaLog(" " + hora_ntp + " - Valor MQ-2: " + String(valorMQ_Novo), logtxt, 2);
      EEPROM.begin(64);
      EEPROM.write(MEM_EEPROM_MQ2, byte(valorMQ_Novo));
      EEPROM.commit();
      LIMITE_MQ2 = byte(EEPROM.read(MEM_EEPROM_MQ2));
      EEPROM.end();

    }
    /*
    			GRAVA PARAMETROS NO SPIFFS(SISTEMA DE ARQUIVO) DA CENTRAL, ARQUIVO "param.txt"
    */
    if (codidoExec == "00012")
    {
      openFS();
      SPIFFS.remove("/param.txt");
      criarArquivo("/param.txt");
      String i = stringUrl;
      stringUrl = "";
      int final_s = i.indexOf("HTTP/1.1");
      stringUrl = i.substring(0, final_s - 1);
      gravaLog(" " + hora_ntp + " - Novos parâmetros da Central", logtxt, 2);
      gravarArquivo("{\"servidor\":\"" + quebraString("servidor", stringUrl) + "\",\"int_1\":\"" + quebraString("int_1", stringUrl) + "\",\"int_2\":\"" + quebraString("int_2", stringUrl) + "\",\"int_3\":\"" + quebraString("int_3", stringUrl) + "\",\"int_4\":\"" + quebraString("int_4", stringUrl) + 
	  "\",\"int_5\":\"" + quebraString("int_5", stringUrl) +"\",\"tipo_1\":\"" + quebraString("tipo_1", stringUrl) + "\",\"tipo_2\":\"" + quebraString("tipo_2", stringUrl) + "\",\"tipo_3\":\"" + quebraString("tipo_3", stringUrl) + "\",\"tipo_4\":\"" + quebraString("tipo_4", stringUrl) +"\",\"tipo_5\":\"" + quebraString("tipo_5", stringUrl) + "\",\"sinal_1\":\"" + quebraString("sinal_1", stringUrl) + "\",\"sinal_2\":\"" + quebraString("sinal_2", stringUrl) + "\",\"sinal_3\":\"" + quebraString("sinal_3", stringUrl) + "\",\"sinal_4\":\"" + quebraString("sinal_4", stringUrl) +"\",\"sinal_5\":\"" + quebraString("sinal_5", stringUrl) + "\",\"log\":\"" + quebraString("log", stringUrl) + "\",\"verao\":\"" + quebraString("verao", stringUrl) + "\",\"nivel\":\"" + quebraString("nivel", stringUrl) + "\",\"senha_alarme\":\"" + quebraString("senhaAlarme", stringUrl) + "\"}", "param.txt");
      cont_ip_banco = 0;
      closeFS();
    }
    /*
    			 APAGAR ARQUIVO DE LOG MANUALMENTE - CHAMADA HTTP EX: HTTP://IP_HOST/?00013
    */
    if (requisicao == "00013")
    {
      deletarArquivo("/log.txt");
      criarArquivo("/log.txt");
    }
    /*
    			APLICAR CONFIGURAÇÕES MINIMAS PARA FUNCIONAMENTO DA CENTRAL - CHAMADA HTTP EX: HTTP://IP_HOST/?00014
    */
    if (requisicao == "00014")
    {
      //SPIFFS.begin(true);
      openFS();
      listDir(SPIFFS, "/", 0);
      //SPIFFS.remove("/param.txt");
      deletarArquivo("/param.txt");
      criarArquivo("/param.txt");
      gravaLog(" " + hora_ntp + " - Configuração minima ", logtxt, 3);
      gravarArquivo("{\"servidor\":\""+String(ipHost)+"\",\"int_1\":\"P1\",\"int_2\":\"P2\",\"int_3\":\"P3\",\"int_4\":\"P4\",\"int_5\":\"P5\",\"tipo_1\":\"0\",\"tipo_2\":\"0\",\"tipo_3\":\"0\",\"tipo_4\":\"0\",\"tipo_5\":\"0\",\"sinal_1\":\"interruptor\",\"sinal_2\":\"interruptor\",\"sinal_3\":\"interruptor\",\"sinal_4\":\"interruptor\",\"sinal_5\":\"interruptor\",\"log\":\"sim\",\"verao\":\"nao\",\"nivel\":\"4\",\"senha_alarme\":\"4\"}", "param.txt");
      closeFS();
    }
    /*
    			DESLIGAR TODOS AS PORTAS OUTPUT DA CENTRAL - CHAMADA HTTP EX: HTTP://IP_HOST/?00015
    */
    if (requisicao == "00015") //
    {
      botao1.contador = 31;
    }
    /*
    			APLICAR AS CONFIGURAÇÕES APÓS SEREM GRAVADAS NA CENTRAL - CHAMADA HTTP EX: HTTP://IP_HOST/?00016
    */
    if (requisicao == "00016") //
    {
      cont_ip_banco = 0;
    }
    /*
    			FUNÇÃO DE CONTROLE DO ALARME
    			ATIVAR ALARME - CHAMADA HTTP EX: HTTP://IP_HOST/?00117
    			DESATIVAR ALARME - CHAMADA HTTP EX: HTTP://IP_HOST/?00017
    */
    if (requisicao == "00117")
    {
      b_status_alarme = 1;
    } else if (requisicao == "00017")
    {
      b_status_alarme = 0;
      alarme.desligado(i_sirene_alarme);
    }
    //---------------------------------------
    //    PAGINA WEB DA CENTRAL
    //---------------------------------------
    String buf;
    /* CABEÇALHO*/
    buf += "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n <!DOCTYPE html><html lang=\"pt-br\">";
    /*HEAD*/
    buf += "<head><meta charset=\"utf-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\"> <link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\">";
    /*STYLE CSS*/
    buf += "<style type=\"text/css\">body .form-control{font-size:12px}input,button,select,optgroup,textarea {  margin: 5px;}.table td, .table th {padding:0px;}.th {width:100px;}.shadow-lg {box-shadow: 0px } #collapseExample {font-size:10px}</style>";
    /*HEAD_END*/
    buf += "<title>Central Automação</title></head><body>";
    /* DIV CONTAINER*/
    buf += "<div class=\"container shadow-lg p-3 mb-5 bg-white rounded\">";
    /*NAV_MENU*/
    buf += "<ul class=\"nav nav-pills mb-3\" id=\"pills-tab\" role=\"tablist\"><li class=\"nav-item\"><a class=\"nav-link active\" id=\"pills-home-tab\" data-toggle=\"pill\" href=\"#pills-home\" role=\"tab\" aria-controls=\"pills-home\" aria-selected=\"true\">Home</a></li> <li class=\"nav-item\"><a class=\"nav-link\" id=\"pills-profile-tab\" data-toggle=\"pill\" href=\"#pills-profile\" role=\"tab\" aria-controls=\"pills-profile\" aria-selected=\"false\">Configuração</a></li> <li class=\"nav-item\"><a class=\"nav-link\" id=\"pills-contact-tab\" data-toggle=\"pill\" href=\"#pills-contact\" role=\"tab\" aria-controls=\"pills-contact\" aria-selected=\"false\">Contato</a></li> </ul>";
    /*DIV PRINCIPAL*/
    buf += "<div class=\"tab-content\" id=\"pills-tabContent\">";
    buf += "<div class=\"tab-pane fade show active\" id=\"pills-home\" role=\"tabpanel\" aria-labelledby=\"pills-home-tab\">";

    buf += "<h4><a href=\"http://" + ipLocalString + "\">CENTRAL -" + ipLocalString + "</a></h4>";
    buf += "<p style=\"text-align:right\"><span class=\"badge badge-pill badge-primary\">Versão: " + VERSAO + "</span></span></p>";
    /*TABLELA_SENSORES*/
    buf += "<table class=\"table table-sm\">";
    buf += "<thead class=\"thead-light\" ><tr><th>SENSOR</th><th>TIPO</th><th>VALOR</th></tr></thead>";
    buf += "<tbody><tr><td>DHT11</td><td>Temperatura/Umidade</td><td>" + String(temperatura) + "Cº / " + String(umidade) + "%</td></tr>";
    buf += "<tr><td>MQ2</td><td>Gás</td><td>" + GLP + " PPM / " + String(sensorMq2) + "</td></tr>";
    buf += "<tr><td>MQ2</td><td>Fumaça</td><td>" + FUMACA + " PPM / " + String(sensorMq2) + "</td></tr>";
    buf += "<tr><td></td><td></td><td></td></tr></tbody>";
    buf += "</table>";
    /*BOTOES_INPUT*/
    buf += "<div><p>";
    if (botao1.estado == true) {
      buf += "  <a href=\"?porta=" + String(botao1.rele) + "&acao=desliga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao1.rele) + " Botão:" + botao1.entrada + "\"><button type=\"button\"  class=\"btn btn-success\">" + String(botao1.nomeInter) + "</button></a>";
    } else {
      buf += "  <a href=\"?porta=" + String(botao1.rele) + "&acao=liga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao1.rele) + " Botão:" + botao1.entrada + "\"><button type=\"button\"  class=\"btn btn-danger\">" + String(botao1.nomeInter) + "</button></a>";
    }
    if (botao2.estado == true) {
      buf += "  <a href=\"?porta=" + String(botao2.rele) + "&acao=desliga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao2.rele) + " Botão:" + botao2.entrada + "\"><button type=\"button\"  class=\"btn btn-success\">" + String(botao2.nomeInter) + "</button></a>";
    } else {
      buf += "  <a href=\"?porta=" + String(botao2.rele) + "&acao=liga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao2.rele) + " Botão:" + botao2.entrada + "\"><button type=\"button\"  class=\"btn btn-danger\">" + String(botao2.nomeInter) + "</button></a>";
    }
    if (botao3.estado == true) {
      buf += "  <a href=\"?porta=" + String(botao3.rele) + "&acao=desliga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao3.rele) + " Botão:" + botao3.entrada + "\"><button type=\"button\"  class=\"btn btn-success\">" + String(botao3.nomeInter) + "</button></a>";
    } else {
      buf += "  <a href=\"?porta=" + String(botao3.rele) + "&acao=liga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao3.rele) + " Botão:" + botao3.entrada + "\"><button type=\"button\"  class=\"btn btn-danger\">" + String(botao3.nomeInter) + "</button></a>";
    }
    if (botao4.estado == true) {
      buf += "  <a href=\"?porta=" + String(botao4.rele) + "&acao=desliga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao4.rele) + " Botão:" + botao4.entrada + "\"><button type=\"button\"  class=\"btn btn-success\">" + String(botao4.nomeInter) + "</button></a>";
    } else {
      buf += "  <a href=\"?porta=" + String(botao4.rele) + "&acao=liga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao4.rele) + " Botão:" + botao4.entrada + "\"><button type=\"button\"  class=\"btn btn-danger\">" + String(botao4.nomeInter) + "</button></a>";
    }
	if (botao5.estado == true) {
      buf += "  <a href=\"?porta=" + String(botao5.rele) + "&acao=desliga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao5.rele) + " Botão:" + botao5.entrada + "\"><button type=\"button\"  class=\"btn btn-success\">" + String(botao5.nomeInter) + "</button></a>";
    } else {
      buf += "  <a href=\"?porta=" + String(botao5.rele) + "&acao=liga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao5.rele) + " Botão:" + botao5.entrada + "\"><button type=\"button\"  class=\"btn btn-danger\">" + String(botao5.nomeInter) + "</button></a>";
    }
    buf += "<a href=\"?00015\" title=\"Desligar\"><button type=\"button\"  class=\"btn btn-danger\">Desli. Tudo</button></a>";
    buf += "</p>";
    /* BOTOES_ALARME */
    if (b_status_alarme == 0) {
      buf += " <tr> <td> <a href=\"?00117\" title=\"Desligar\"> <button type=\"button\" class=\"btn btn-success\">Ligar Alarme</button> </a> </td> </tr> ";
    } else {
      buf += " <tr> <td> <div class=\"accordion\" id=\"accordionExample\"> <button class=\"btn btn-danger\" type=\"button\" data-toggle=\"collapse\" data-target=\"#collapseTwo\" aria-expanded=\"false\" aria-controls=\"collapseTwo\"> Desli. Alarme </button> <div id=\"collapseTwo\" class=\"collapse\" aria-labelledby=\"headingTwo\" data-parent=\"#accordionExample\"> <form name=\"calcform\" method=\"post\" action=\"\"> <fieldset> <input type=\"password\" name=\"visor\" id=\"visor\" /> <table id=\"calc\"> <tr> <td> <input type=\"button\" name=\"num1\" class=\"num\" value=\"1\" onclick=\"calcNum(1)\" /> </td> <td> <input type=\"button\" name=\"num2\" class=\"num\" value=\"2\" onclick=\"calcNum(2)\" /> </td> <td> <input type=\"button\" name=\"num3\" class=\"num\" value=\"3\" onclick=\"calcNum(3)\" /> </td> </tr> <tr> <td> <input type=\"button\" name=\"num4\" class=\"num\" value=\"4\" onclick=\"calcNum(4)\" /> </td> <td> <input type=\"button\" name=\"num5\" class=\"num\" value=\"5\" onclick=\"calcNum(5)\" /> </td> <td> <input type=\"button\" name=\"num6\" class=\"num\" value=\"6\" onclick=\"calcNum(6)\" /> </td> </tr> <tr> <td> <input type=\"button\" name=\"num7\" class=\"num\" value=\"7\" onclick=\"calcNum(7)\" /> </td> <td> <input type=\"button\" name=\"num8\" class=\"num\" value=\"8\" onclick=\"calcNum(8)\" /> </td> <td> <input type=\"button\" name=\"num9\" class=\"num\" value=\"9\" onclick=\"calcNum(9)\" /> </td> </tr> <tr> <td> <input type=\"button\" name=\"limpar\" class=\"num\" value=\"X\" onclick=\"calcLimpar()\" /> </td> <td> <input type=\"button\" name=\"num0\" class=\"num\" value=\"0\" onclick=\"calcNum(0)\" /> </td> <td> <input type=\"button\" name=\"igual\" class=\"num\" value=\"=\" onclick=\"calcParse('resultado')\" /> </td> </tr></table> </fieldset> </form> </div> </div> </td> </tr> ";
    }
    buf += "</div></div>";
    /*DIV CONFIGURAÇÕES*/
    buf += "<div class=\"tab-pane fade\" id=\"pills-profile\" role=\"tabpanel\" aria-labelledby=\"pills-profile-tab\">";
    buf += "<h4>Configurar Sensor</h4>";
    buf += "<form class=\"form-group\" method=\"get\"><table class=\"table-responsive\" style=\"width:100%\"><tr><td><input type=\"hidden\" name=\"cod\" value=\"00011\"><label for=\"inputEmail4\">Limite Sensor Gás: </label></td><td><input class=\"form-control mb-2\" style=\"width:50px\" type=\"text\" placeholder=\"\" name=\"valor\" value=\"" + String(LIMITE_MQ2) + "\"></td><td> <input class=\"btn btn-info\" type=\"submit\" value=\"Alterar\"><a href=\"?00010\"></td><td><button class=\"btn btn-warning\" type=\"button\"  >Recalibrar</button></a></td></tr></table></form>";
    buf += "<h4>Parâmetros Gerais</h4>";
    buf += "<form class=\"form-group\" action=\"?00012\"><table class=\"table-responsive\"><input type=\"hidden\" name=\"cod\" value=\"00012\">";
    buf += "<tr><td ><label for=\"inputEmail4\">Servidor</label> </td><td colspan=\"3\"><input class=\"form-control mb-2\" style=\"width:130px\" type=\"text\" placeholder=\"\" name=\"servidor\" value=\"" + serv + "\"></td></tr>";
    buf += "<tr><td ><label for=\"inputEmail4\">IP Fixo</label> </td><td colspan=\"3\"><input class=\"form-control mb-2\" style=\"width:130px\" type=\"text\" placeholder=\"\" name=\"ipFixo\" value=\"" + IP_FIXO + "\"></td></tr>";
    
    buf += "<tr><td ><label for=\"inputEmail4\">Gateway</label> </td><td colspan=\"3\"><input class=\"form-control mb-2\" style=\"width:130px\" type=\"text\" placeholder=\"\" name=\"servidor\" value=\"" + GATEWAY + "\"></td></tr>";
    buf += "<tr><td ><label for=\"inputEmail4\">Mascara</label> </td><td colspan=\"3\"><input class=\"form-control mb-2\" style=\"width:130px\" type=\"text\" placeholder=\"\" name=\"servidor\" value=\"" + MASCARA_IP + "\"></td></tr>";
    buf += "<tr>";
    buf += "<td><label for=\"inputEmail4\">Interruptor 1</label></td><td><input class=\"form-control mb-2\" style=\"width:100%x\" type=\"text\" placeholder=\"\" name=\"int_1\" value=\"" + String(botao1.nomeInter) + "\"></td>";
    buf += "<td><select class=\"form-control mb-2\" style=\"width:100%x\"  name=\"tipo_1\"><option value=\"0\" " + selectedHTNL(botao1.tipo, "0") + "> Negativo</option><option value=\"1\" " + selectedHTNL(botao1.tipo, "1") + "> Positivo</option></select></td>";
    buf += "<td><select class=\"form-control mb-2\" style=\"width:100%x\" name=\"sinal_1\"><option value=\"pulso\" " + selectedHTNL(botao1.modelo, "pulso") + "> Pulso</option><option value=\"interruptor\" " + selectedHTNL(botao1.modelo, "interruptor") + ">Interruptor</option><option value=\"pir\" " + selectedHTNL(botao1.modelo, "pir") + ">PIR</option></select></td></tr>";
    buf += "<tr><td><label for=\"inputEmail4\">Interruptor 2</label></td><td><input class=\"form-control mb-2\" style=\"width:100%x\" type=\"text\" placeholder=\"\" name=\"int_2\" value=\"" + String(botao2.nomeInter) + "\"></td>";
    buf += "<td><select class=\"form-control mb-2\" style=\"width:100%x\"  name=\"tipo_2\"><option value=\"0\" " + selectedHTNL(botao2.tipo, "0") + " > Negativo</option><option value=\"1\" " + selectedHTNL(botao2.tipo, "1") + "> Positivo</option></select></td>";
    buf += "<td><select class=\"form-control mb-2\" style=\"width:100%x\" name=\"sinal_2\"><option value=\"pulso\" " + selectedHTNL(botao2.modelo, "pulso") + "> Pulso</option><option value=\"interruptor\" " + selectedHTNL(botao2.modelo, "interruptor") + ">Interruptor</option><option value=\"pir\" " + selectedHTNL(botao2.modelo, "pir") + ">PIR</option></select></td></tr>";
    buf += "<tr><td><label for=\"inputEmail4\">Interruptor 3</label></td><td><input class=\"form-control mb-2\" style=\"width:100%x\" type=\"text\" placeholder=\"\" name=\"int_3\" value=\"" + String(botao3.nomeInter) + "\"></td>";
    buf += "<td><select class=\"form-control mb-2\" style=\"width:100%x\"  name=\"tipo_3\"><option value=\"0\" " + selectedHTNL(botao3.tipo, "0") + "> Negativo</option><option value=\"1\" " + selectedHTNL(botao3.tipo, "1") + "> Positivo</option></select></td>";
    buf += "<td><select class=\"form-control mb-2\" style=\"width:100%x\" name=\"sinal_3\"><option value=\"pulso\" " + selectedHTNL(botao3.modelo, "pulso") + "> Pulso</option><option value=\"interruptor\" " + selectedHTNL(botao3.modelo, "interruptor") + ">Interruptor</option><option value=\"pir\" " + selectedHTNL(botao3.modelo, "pir") + ">PIR</option></select></td></tr>";
    buf += "<tr><td><label for=\"inputEmail4\">Interruptor 4</label></td><td><input class=\"form-control mb-2\" style=\"width:100%x\" type=\"text\" placeholder=\"\" name=\"int_4\" value=\"" + String(botao4.nomeInter) + "\"></td>";
    buf += "<td><select class=\"form-control mb-2\" style=\"width:100%x\"  name=\"tipo_4\"><option value=\"0\" " + selectedHTNL(botao4.tipo, "0") + "> Negativo</option><option value=\"1\" " + selectedHTNL(botao4.tipo, "1") + "> Positivo</option></select></td>";
    buf += "<td><select class=\"form-control mb-2\" style=\"width:100%x\" name=\"sinal_4\"><option value=\"pulso\" " + selectedHTNL(botao4.modelo, "pulso") + "> Pulso</option><option value=\"interruptor\" " + selectedHTNL(botao4.modelo, "interruptor") + ">Interruptor</option><option value=\"pir\" " + selectedHTNL(botao4.modelo, "pir") + ">PIR</option></select></td></tr>";
    buf += "<tr><td><label for=\"inputEmail4\">Interruptor 5</label></td><td><input class=\"form-control mb-2\" style=\"width:100%x\" type=\"text\" placeholder=\"\" name=\"int_5\" value=\"" + String(botao5.nomeInter) + "\"></td>";
    buf += "<td><select class=\"form-control mb-2\" style=\"width:100%x\"  name=\"tipo_5\"><option value=\"0\" " + selectedHTNL(botao5.tipo, "0") + "> Negativo</option><option value=\"1\" " + selectedHTNL(botao5.tipo, "1") + "> Positivo</option></select></td>";
    buf += "<td><select class=\"form-control mb-2\" style=\"width:100%x\" name=\"sinal_5\"><option value=\"pulso\" " + selectedHTNL(botao5.modelo, "pulso") + "> Pulso</option><option value=\"interruptor\" " + selectedHTNL(botao5.modelo, "interruptor") + ">Interruptor</option><option value=\"pir\" " + selectedHTNL(botao5.modelo, "pir") + ">PIR</option></select></td></tr>";
	
	// Config de log
    buf += "<tr> <td> <label for='inputEmail4'>Registro de log</label> </td> ";
    buf += "<td> <select class='form-control mb-2' style='width:100%x' name='log'> <option value='sim' " + selectedHTNL(conslog, "sim ") + "> Sim</option> <option value='nao' " + selectedHTNL(conslog, "nao ") + ">Não</option> </select> </td> <td> <select class='form-control mb-2' style='width:100%x' name='nivel' title='Nível do log'> <option value='1' " + selectedHTNL(nivelLog, "1") + ">1</option> <option value='2' " + selectedHTNL(nivelLog, "2") + ">2</option> <option value='3' " + selectedHTNL(nivelLog, "3") + ">3</option> <option value='4' " + selectedHTNL(nivelLog, "4") + ">4</option> </select> </td> </tr> ";
    /*
      CONFIG. DO ALARME
    */
    buf += "<tr><td><label for=\"inputEmail4\">Senha Alarme</label></td><td><input class=\"form-control mb-2\" style=\"width:100%x\" type=\"password\" placeholder=\"\" name=\"senhaAlarme\" value=\"" + String(s_senha_alarme) + "\"></td><td><input class='btn btn-info' type='submit' value='Salvar'></td> </tr>";
    // Botao Salvar Config
    buf += "<tr> <td><a href='?00013' title='Apagar log'><button type='button' class='btn btn-danger'>Deletar Log</button></td> <td> <button class='btn btn-primary' type='button' data-toggle='collapse' data-target='#collapseExample' aria-expanded='false' aria-controls='collapseExample'>Ver Logs </button></td> <td> </td> <td> </td> </tr> ";
    buf += "</table> </form> ";
    buf += "<div class='collapse' id='collapseExample'> <div class='card card-body'> ";
    buf += lerArquivo();
    buf += "</div> </div> </div> </div> ";
    buf += "<script src=\"https://code.jquery.com/jquery-3.3.1.slim.min.js\" integrity=\"sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo\" crossorigin=\"anonymous\"></script>";
    buf += "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.3/umd/popper.min.js\" integrity=\"sha384-ZMP7rVo3mIykV+2+9J3UJ46jBk0WLaUAdn689aCwoqbBJiSnjAK/l8WvCWPIPm49\" crossorigin=\"anonymous\"></script>";
    buf += "<script src=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/js/bootstrap.min.js\" integrity=\"sha384-ChfqqxuZUCnJSK3+MXmPNIyE6ZbWh2IMqE241rYiqJxyMiZ6OW/JmZQ5stwEULTy\" crossorigin=\"anonymous\"></script>";
    buf += "</body></html>";
    buf += " <script> function calcNum(num) { document.calcform.visor.type = 'password'; document.calcform.visor.value = document.calcform.visor.value + num; } var delay = 1500;  function calcParse(oper) { var valor = document.calcform.visor.value; if (valor == '') { document.calcform.visor.value = ''; } else { var senha = '" + String(s_senha_alarme) + "'; document.calcform.visor.type = 'text'; if (senha == valor) { document.calcform.visor.value = 'Senha Correta       '; window.location.href = \"?00017\"; valor = ''; } else { document.calcform.visor.value = 'Senha Incorreta      '; valor = ''; setTimeout(function() { calcLimpar(); }, delay); } } }  function calcLimpar() { document.calcform.visor.value = ''; } </script> ";
    buf += " </body>  </html> ";
    client.print(buf);
    client.flush();
    client.stop();
  }
  /*
    ALARME
  */
  alarme.monitoramento(i_sensor_alarme, i_sirene_alarme, b_status_alarme);

  //---------------------------------------
  //    ROTINA DO SENSOR MQ-2
  //---------------------------------------
  if (millis() >= timeMq2 + (timeMq2Param * 1)) {
    sensorMq2 = analogRead(PIN_MQ2);
   // if (sensorMq2 < 5000) {
      GLP = String(getQuantidadeGasMQ(leitura_MQ2(PIN_MQ2) / Ro, GAS_LPG) );
      //if (GLP == "2147483647") GLP = "0";
      FUMACA = String(getQuantidadeGasMQ(leitura_MQ2(PIN_MQ2) / Ro, SMOKE));
      //if (FUMACA == "2147483647") FUMACA = "0";
      String CO = String(getQuantidadeGasMQ(leitura_MQ2(PIN_MQ2) / Ro, GAS_CO)  );
      //if (CO == "2147483647") CO = "0";
      contarParaGravar1++;
      gravaLog(" " + hora_ntp + " - MQ2 A: " + String(sensorMq2) + " GLP:" + GLP + " " + "CO:" + CO + " " + "FU:" + FUMACA + " " + "L:" + contarParaGravar1, logtxt, 4);
//    } else {
//      //Gravando log de erro na central.
//      for (int i = 0; i <= 0 ; i++ )
//      {
//        gravaLog(" " + hora_ntp + " - ERRO 0103 - Sensor MQ-2", logtxt, 1);
//        GLP = "0";
//      }
//    }
    timeMq2 = millis();
    buff = "sensor=mq-2&valor=mq-2;" + String(GLP) + ";&central=" + String(ipLocalString) + "&p=" + String(PIN_MQ2);
    if (GLP > LIMITE_MQ2)
    {
      sirene(true);
      pisca_led(LED_VERDE, false);
      pisca_led(LED_VERMELHO, true);
    } else
    {
      sirene(false);
      pisca_led(LED_VERMELHO, false);
    }
    //GRAVA NO BANCO O VALOR LIDO APOS X LEITURAS
    if ((contarParaGravar1 == 20) || (GLP >= LIMITE_MQ2))
    {
      gravarBanco (buff);
      contarParaGravar1 = 0;
    }
  }

  //---------------------------------------
  //    ROTINA DO SENSOR DHT11
  //---------------------------------------
  umidade = dht.readHumidity() * 1;
  temperatura = dht.readTemperature() * 1;
  if (millis() >= timeDht + (timeDhtParam)) {
    umidade = dht.readHumidity();
    temperatura = dht.readTemperature();
    int t = 1;
    if ((temperatura == int(temperatura)) && (umidade == int(umidade)) && (t == 1) )
    {
      timeDht = millis();
      buff = "sensor=dht11&valor=dht11;" + String(temperatura) + ";" + String(umidade) + ";&central=" + String(ipLocalString) + "&p=" + String(DHTPIN);
      gravarBanco(buff);
    } else {
      gravaLog(" " + hora_ntp + " - ERRO 0109 - Sensor DHT", logtxt, 1);
      t = 0;
      temperatura = 0;
      umidade = 0;
      timeDht = millis();
    }
  }

  //---------------------------------------

  //---------------------------------------
  //     FIM DA FUNÇÃO LOOP
  //---------------------------------------
}
