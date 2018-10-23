//---------------------------------------
//    INCLUINDO BIBLIOTECAS
//---------------------------------------
#include <WiFi.h>         
#include <DNSServer.h> 
#include <WebServer.h> 
#include <WiFiManager.h>
#include <DHT.h>
#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUDP.h>
#include <SPIFFS.h>
#include <FS.h>
#include <ArduinoJson.h>
//---------------------------------------
//flag para indicar se foi salva uma nova configuração de rede
//bool shouldSaveConfig = false;

//---------------------------------------
//    DEFINIÇÕES DE VARIAVEIS FIXAS
//---------------------------------------
#define BUZZER      5
#define LEDVERDE    16
#define LEDVERMELHO 17
#define PIN_MQ2     34
#define DHTPIN      32
#define DHTTYPE     DHT11
#define VRL_VALOR   5                 //resistência de carga
#define RO_FATOR_AR_LIMPO     9.83    //resistência do sensor em ar limpo 9.83 de acordo com o datasheet
#define ITERACOES_CALIBRACAO  40      //numero de leituras para calibracao
#define ITERACOES_LEITURA     10       //numero de leituras para analise
#define GAS_LPG   0  
#define GAS_CO    1
#define SMOKE     2
//---------------------------------------

//---------------------------------------
//    INSTANCIANDO STRUCT'S
//---------------------------------------
struct botao1{
	int entrada = 27, rele = 26;
	boolean estado = 0, tipo;
	char contador = 0;
	String modelo = "pulso";
}botao1;
struct botao2{
	int entrada = 19, rele = 23;
	boolean estado = 0, tipo;
	char contador = 0;
}botao2;
struct botao3{
	int entrada = 14, rele = 12;
	boolean estado = 0, tipo;
	char contador = 0;
}botao3;
struct botao4{
  int entrada = 25, rele =15;
  boolean estado = 0, tipo;
  char contador = 0;
}botao4;
//---------------------------------------

//---------------------------------------
//    INICIANDO VARIAVEIS
//---------------------------------------
String versao = "V5 - 19/10/2018";
const char* ssid;
const char* password;
//byte servidor[] = {192,168,0,20};
const char *servidor;
char portaServidor = 80, contarParaGravar1 = 0, contarParaGravar2 = 0 ;
int sensorMq2 = 0, contadorPorta = 0, LED_BUILTIN = 2;
int T_WIFI = 50;              // DELAY PARA DAR TEMPO DA PLACA CONECTAR NO WIFI
int REINICIO_CENTRAL, MEM_EEPROM_C = 5 , MEM_EEPROM_1 = 7,MEM_EEPROM_2 = 9, MEM_EEPROM_3 = 12,MEM_EEPROM_4 = 14, MEM_EEPROM_MQ2 = 20;
short paramTempo = 60;
String LIMITE_MQ2, buf;
float umidade = 0, temperatura = 0;
unsigned long time3, time3Param = 100000, timeDht, timeMq2 , tempo = 0, timeDhtParam = 300000, timeMq2Param = 10000;
IPAddress ipHost;
String ipLocalString, buff, URL, linha;
boolean ultimoStatus = 0, atualStatus = 0 ;
WiFiUDP udp;//Cria um objeto "UDP".
NTPClient ntp(udp, "c.st1.ntp.br", -3 * 3600, 60000);//Cria um objeto "NTP" com as configurações.
String hora;
int nContar = 0;
//pino do botão reset das configurações wifi
const int PIN_AP = 3;
float LPGCurve[3]  =  {2.3,0.20,-0.47};   //curva LPG aproximada baseada na sensibilidade descrita no datasheet {x,y,deslocamento} baseada em dois pontos 
//p1: (log200, log1.6), p2: (log10000, log0.26)
//inclinacao = (Y2-Y1)/(X2-X1)
//vetor={x, y, inclinacao}
float COCurve[3]  =  {2.3,0.72,-0.34};    //curva CO aproximada baseada na sensibilidade descrita no datasheet {x,y,deslocamento} baseada em dois pontos 
//p1: (log200, 0.72), p2(log10000, 0.15)
//inclinacao = (Y2-Y1)/(X2-X1)
//vetor={x, y, inclinacao}
float SmokeCurve[3] ={2.3,0.53,-0.44};    //curva LPG aproximada baseada na sensibilidade descrita no datasheet {x,y,deslocamento} baseada em dois pontos 
//p1: (log200, 0.53), p2: (log10000, -0.22)
//inclinacao = (Y2-Y1)/(X2-X1)
//vetor={x, y, inclinacao}
float Ro = 10; 
String GLP;
String FUMACA; 
String retorno;
int cont_ip_banco = 0;
int nivel_log = 1; // 1,2,3,4
int estado_atual = 0;
int estado_antes = 0;
//---------------------------------------

//---------------------------------------
//    INSTANCIANDO CLASSES
//---------------------------------------
DHT dht(DHTPIN, DHTTYPE);
WiFiServer server(80);
//---------------------------------------

void setup() {
	Serial.begin(115200);
	EEPROM.begin(64);
	dht.begin();
	
	pinMode(PIN_AP, INPUT_PULLUP);
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN,HIGH);
	
	pinMode(botao1.rele,OUTPUT);
	digitalWrite(botao1.rele,LOW);
	pinMode(botao1.entrada,INPUT_PULLUP);

	pinMode(botao2.rele,OUTPUT);
	pinMode(botao2.entrada,INPUT_PULLUP);
	digitalWrite(botao2.rele,LOW);

	pinMode(botao3.rele,OUTPUT);
	pinMode(botao3.entrada,INPUT_PULLUP);
	digitalWrite(botao2.rele,LOW);

	pinMode(botao4.rele,OUTPUT);
	pinMode(botao4.entrada,INPUT_PULLUP);
	digitalWrite(botao4.rele,LOW);
  
	pinMode(0, INPUT);

	pinMode(BUZZER, OUTPUT); 
	digitalWrite(BUZZER,HIGH);

	pinMode(LEDVERDE, OUTPUT);
	pinMode(LEDVERMELHO, OUTPUT);
	
	//---------------------------------------  
	//    LOG
	//---------------------------------------
	//SPIFFS.begin(); 
	//openFS();
	criarArquivo();
	//gravarArquivo("{\"servidor\":\"192.168.0.20\",\"porta\":\"80\",\"int_1\":\"Interruptor 1\",\"int_2\":\"Interruptor 2\",\"int_3\":\"Interruptor 3\,\"int_4\":\"Interruptor 4\"}","param.txt");
	/* StaticJsonDocument<400> doc;
	const char* json = lerArquivoParam().c_str();
	//const char* json = f;
	DeserializationError error = deserializeJson(doc, json);
	// Test if parsing succeeds.
	if (error) {
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.c_str());
		return;
	}
	JsonObject root = doc.as<JsonObject>();
	servidor = root["servidor"];
	Serial.println(servidor); */
 
	//---------------------------------------  
	//    CONECTANDO A REDE WIFI
	//---------------------------------------
	Serial.println("");
	Serial.print(" Conectado a rede WIFI  ");
	WiFiManager wifiManager;
	wifiManager.setAPCallback(configModeCallback); 
	wifiManager.setSaveConfigCallback(saveConfigCallback); 
	wifiManager.autoConnect("ESP_AP","12345678"); //cria uma rede sem senha
	server.begin();
	ipHost = WiFi.localIP()
	ipLocalString = String(ipHost[0])+"."+String(ipHost[1])+"."+String(ipHost[2])+"."+String(ipHost[3]);
	//Serial.println(ipHost);
	//LÊ ENDEREÇO NA EEPROM CORRESPONDENTE AO PARAMETRO DE REINICI DA CENTRAL
	REINICIO_CENTRAL = EEPROM.read(MEM_EEPROM_C);   
	//--------------------------------------- 
	//LEITUTA DO TIPO DE INTERRUPTOR QUE ESTA EM USO NA CENTRAL
	// PORDE SER HIGH OU LOW
	//--------------------------------------- 	
	botao1.tipo = byte(EEPROM.read(MEM_EEPROM_1));
	botao2.tipo = byte(EEPROM.read(MEM_EEPROM_2));
	botao3.tipo = byte(EEPROM.read(MEM_EEPROM_3));
	botao4.tipo = byte(EEPROM.read(MEM_EEPROM_4));
  
	Serial.println("----------------------------------");
	Serial.println("Configurações da Central:");
	Serial.println("IP da Central: "+ ipLocalString);
	Serial.println(" Interruptor 1  com sinal: "+String(botao1.tipo));
	Serial.println(" Interruptor 2, com sinal: "+String(botao2.tipo));
	Serial.println(" Interruptor 3, com sinal: "+String(botao3.tipo));
	Serial.println(" Interruptor 4, com sinal: "+String(botao4.tipo));
	//Serial.println("----------------------------------");
	//--------------------------------------- 
	
	//--------------------------------------- 
	// PARAMETROS DA MEMORIA EEPROM
	//--------------------------------------- 
	LIMITE_MQ2 = byte(EEPROM.read(MEM_EEPROM_MQ2));
	Serial.println("");
	//Serial.println("----------------------------------");
	Serial.println("=> Valor sensor de Gás e GLP: "+String(LIMITE_MQ2));
	Serial.println("----------------------------------");
	//--------------------------------------- 
	
	//PRIMEIRA LEITURA DO SENSORES
	//---------------------------------------
	//DHT11
	umidade = dht.readHumidity() * 1;
	temperatura = dht.readTemperature() * 1;
	//MQ2
	sensorMq2 = analogRead(PIN_MQ2);
	GLP = String(getQuantidadeGasMQ(leitura_MQ2(PIN_MQ2)/Ro,GAS_LPG) );
	FUMACA = String(getQuantidadeGasMQ(leitura_MQ2(PIN_MQ2)/Ro,SMOKE));
	//---------------------------------------  
	
	//---------------------------------------   
	//CALIBRAR LEITURA DO SENSOR DE GAS E FUMACA
	//--------------------------------------- 
	calibrarSensor();
	//--------------------------------------- 
	EEPROM.end();
	retorno = "SERVIDOR_CONECT";
}

void loop() {
  WiFiManager wifiManager;
  WiFiClient client = server.available();
  hora = ntp.getFormattedTime();
	
	while(cont_ip_banco < 1)
	{
		//Gravando no log o reinicio da central
		gravarArquivo(hora+"Central Reiniciada...Conectado a Rede Wifi: "+String(ssid),"log.txt");
		//Leitura de parametros da central
		StaticJsonDocument<400> doc;
		const char* json = lerArquivoParam().c_str();
		DeserializationError error = deserializeJson(doc, json);
		if (error) 
		{
			Serial.print(F(" ERRO 0101 - Falha ao desenraizar o arquivo json: "));
			//Gravando log de erro na central.
			if (nivel_log >= 1) gravarArquivo( hora+" - ERRO 0101","log.txt");
			Serial.println(error.c_str());
			return;
		}
		JsonObject root = doc.as<JsonObject>();
		servidor = root["servidor"];
		cont_ip_banco++;
	}
  
  //se o botão foi pressionado, reseta as configurações WIFI da central
   if ( digitalRead(PIN_AP) == LOW ) 
	 {
		//Apagando dados de conexão WIFI da central
		//Gravando log de erro na central.
		if (nivel_log >= 1) gravarArquivo( hora+" - Entrando em modo 'AP' e apagando configurações WIFI ","log.txt");
	  esp_wifi_restore(); 
	  Serial.println(" Apagando configurações WIFI..."); //tenta abrir o portal
		delay(2000);
    if(!wifiManager.startConfigPortal("ESP_AP", "12345678") )
		{
			Serial.println(" ERRO 0102 - Falha ao conectar no WIFI modo AP (Access Poin)");
			if (nivel_log >= 1) gravarArquivo( hora+" - ERRO 0102","log.txt");
        delay(2000);
        ESP.restart();
        delay(1000);
      }
		Serial.println(">= Central em modo de configuração do WIFI...");
   }
   
	//---------------------------------------
	//    ENTRADA E SAIDA 1
	//---------------------------------------
  if(botao1.modelo == "pulso")
	{
		if(digitalRead(botao1.entrada) == botao1.tipo)
		{
			while((digitalRead(botao1.entrada) == botao1.tipo) && (nContar <= 100) )
			{
				if(millis() >= tempo + paramTempo)
				{ 
					botao1.contador++;
					nContar++;
					Serial.print(" Modo pulso... "+botao1.contador,DEC);
					tempo = millis();   
				}
			}
		}
	}else if(botao1.modelo == "interruptor")
	{
		estado_atual = digitalRead(botao1.entrada);
		if(estado_atual <> estado_antes )
		{
			estado_antes = estado_atual;
			botao1.contador = 1;
			Serial.print(" Modo Interruptor... "+botao1.contador,DEC);
		}
	}else if(botao1.modelo == "temporario")
	{
			Serial.print(" Modo Temporario: "+botao1.contador,DEC);
			Serial.println("=> LIGANDO RELE 1 - porta: "+String(botao1.rele));
			botao1.estado = true;
			botao1.contador = 0;
			acionaPorta(botao1.rele,"","liga");
			delay(1000);
			Serial.println("=> DESLIGANDO RELE 1 - porta: "+String(botao1.rele));
			acionaPorta(botao1.rele,"","desl");
			botao1.estado = false;
	}
	if((botao1.contador >= 1) && (botao1.contador <= 9))
	{
		if(nContar >= 100)
     {
			String ERRO_ENTRADA = " ERRO 0107 - Botão 1 com erro!";
			//Gravando log de erro na central.
			if (nivel_log >= 1) gravarArquivo( hora+" - ERRO 0107","log.txt");
     }else{
				String ERRO_ENTRADA = "0";
				nContar = 0;
    		if(botao1.estado == false){
    			Serial.println("=> LIGANDO RELE 1 - porta: "+String(botao1.rele));
    			botao1.estado = true;
    			botao1.contador = 0;
    			acionaPorta(botao1.rele,"","liga");
    		}else{
    			Serial.println("=> DESLIGANDO RELE 1 - porta: "+String(botao1.rele));
    			acionaPorta(botao1.rele,"","desl");
    			botao1.estado = false;
    			botao1.contador = 0;
    		}
    	}
	}
	//---------------------------------------
	
	//---------------------------------------
	//    ENTRADA E SAIDA 2
	//---------------------------------------
	if(digitalRead(botao2.entrada) ==  botao2.tipo){
		while((digitalRead(botao2.entrada) ==  botao2.tipo)&& (nContar <= 100)){
			if(millis() >= tempo + paramTempo){ 
				botao2.contador++;
        nContar++;
				Serial.print(botao2.contador,DEC);
				tempo = millis();   
			}
		}
	}
	if((botao2.contador >= 1) && (botao2.contador <= 9)){
		  if(nContar >= 100)
     {
      String ERRO_ENTRADA = "Botão 2 com erro!";
			
     }else{
			String ERRO_ENTRADA = "0";
			nContar = 0;
			if(botao2.estado == false){
			Serial.println("=> LIGANDO RELE 2 - porta: "+String(botao2.rele));
			botao2.estado = true;
			botao2.contador = 0;
			acionaPorta(botao2.rele,"","liga");
		}else{
			Serial.println("=> DESLIGANDO RELE 2 - porta: "+String(botao2.rele));
			acionaPorta(botao2.rele,"","desl");
			botao2.estado = false;
			botao2.contador = 0;
		}
	}
	}
	//---------------------------------------
	
	//---------------------------------------
	//    ENTRADA E SAIDA 3
	//---------------------------------------
	/* if(digitalRead(botao3.entrada) == botao3.tipo){
		while((digitalRead(botao3.entrada) == botao3.tipo)&& (nContar <= 100)){
			if(millis() >= tempo + paramTempo){ 
				botao3.contador++;
        nContar++;
				Serial.print(botao3.contador,DEC);
				tempo = millis();   
			}
		}
	}
	if((botao3.contador >= 1) && (botao3.contador <= 9)){
    if(nContar >= 100)
     {
      String ERRO_ENTRADA = "Botão 3 com erro!";
     }else{
			String ERRO_ENTRADA = "0";
			nContar = 0;
		if(botao3.estado == false){
			Serial.println("=> LIGANDO RELE 3 - porta: "+String(botao3.rele));
			botao3.estado = true;
			botao3.contador = 0;
			acionaPorta(botao3.rele,"","liga");
		}else{
			Serial.println("=> DESLIGANDO RELE 3- porta: "+String(botao3.rele));
			acionaPorta(botao3.rele,"","desl");
			botao3.estado = false;
			botao3.contador = 0;
		}
	} 
	}
	//---------------------------------------

  //---------------------------------------
  //    ENTRADA E SAIDA 4
  //---------------------------------------
  if(digitalRead(botao4.entrada) == botao4.tipo){
    while((digitalRead(botao4.entrada) == botao4.tipo)&& (nContar <= 100)){
      if(millis() >= tempo + paramTempo){ 
        botao4.contador++;
        nContar++;
        Serial.print(botao4.contador,DEC);
        tempo = millis();   
      }
    }
  }
  if((botao4.contador >= 1) && (botao4.contador <= 9)){
        if(nContar >= 100)
     {
      String ERRO_ENTRADA = "Botão 4 com erro!";
     }else{
			 String ERRO_ENTRADA = "0";
			 nContar = 0;
    if(botao4.estado == false){
      Serial.println("=> LIGANDO RELE 4 - porta: "+String(botao4.rele));
      botao4.estado = true;
      botao4.contador = 0;
      acionaPorta(botao4.rele,"","liga");
    }else{
      Serial.println("=> DESLIGANDO RELE 4- porta: "+String(botao4.rele));
      acionaPorta(botao4.rele,"","desl");
      botao4.estado = false;
      botao4.contador = 0;
    }
  }}  */
  //---------------------------------------
	
	//---------------------------------------
	//    LIGAR E DESLIGAR TODOS RELES
	//---------------------------------------
	if(((botao1.contador >=10) && (botao1.contador <= 20)) 
			|| ((botao2.contador >=10) && (botao2.contador <= 20)) 
			|| ((botao3.contador >=10) && (botao3.contador <= 20))
      || ((botao4.contador >=10) && (botao4.contador <= 20)) )
	{
		Serial.println(">= DESLIGANDO TODOS OS RELES");
		acionaPorta(botao1.rele,"","desl");
		//digitalWrite(botao1.rele,LOW);
		botao1.estado = false;
		acionaPorta(botao2.rele,"","desl");
		//digitalWrite(botao2.rele,LOW);
		botao2.estado = false;
		acionaPorta(botao3.rele,"","desl");
		//digitalWrite(botao3.rele,LOW);
		botao3.estado = false;
		acionaPorta(botao4.rele,"","desl");
		//digitalWrite(botao4.rele,LOW);
		botao4.estado = false;
		botao4.contador = 0;
		botao3.contador = 0;
		botao2.contador = 0;
		botao1.contador = 0;
	}
	if((botao1.contador >=20) || (botao2.contador >=20) || (botao3.contador >=20) || (botao4.contador >=20)){
		Serial.println(">= LIGANDO TODOS OS RELES");
		acionaPorta(botao1.rele,"","liga");
			//digitalWrite(botao1.rele,HIGH);
		botao1.estado = true;
		acionaPorta(botao2.rele,"","liga");
		//digitalWrite(botao2.rele,HIGH);
		botao2.estado = true;
		acionaPorta(botao3.rele,"","liga");
		//digitalWrite(botao3.rele,HIGH);
		botao3.estado = true;
		acionaPorta(botao4.rele,"","liga");
		//digitalWrite(botao4.rele,HIGH);
		botao4.estado = true;
		botao4.contador = 0;
		botao3.contador = 0;
		botao2.contador = 0;
		botao1.contador = 0;
	}

	//---------------------------------------
	//    LEITURA DA REQUISIÇÃO GET
	//---------------------------------------
	if (client) {
		URL = "";
		URL = client.readStringUntil('\r');
		Serial.print("=> RECEBIDO DADOS : ");
		Serial.println(URL);
	}else{
		URL = "vazio";
	}
	if(URL != "vazio"){
		//EXEMPLO NA CHAMADA WEB DESLIGAR LAMPADA - ?porta=20&acao=desligar&central=192.168.0.177
		String stringUrl = URL;
		URL = "";
		String requisicao = stringUrl.substring(6, 11);
		Serial.println(requisicao);
		if (requisicao == "porta") {
			String numero = stringUrl.substring(12, 14);
			String acao = stringUrl.substring(20, 24);
			String central = stringUrl.substring(33, 40);
			int numeroInt = numero.toInt();
			acionaPorta(numeroInt, requisicao, acao);
			if(numeroInt == botao1.rele){
				if(acao == "liga"){
					botao1.estado = true;
				}else{
					botao1.estado = false;
				}
			}else if(numeroInt == botao2.rele){
				if(acao == "liga"){
					botao2.estado = true;
				}else{
					botao2.estado = false;
				}
			}else if(numeroInt == botao3.rele){
				if(acao == "liga"){
					botao3.estado = true;
				}else{
					botao3.estado = false;
				}
			}else if(numeroInt == botao4.rele){
       if(acao == "liga"){
          botao4.estado = true;
        }else{
          botao4.estado = false;
        }  
			}
/* 			String buff;
			buff += "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://"+String(servidor[0])+"."+String(servidor[1])+"."+String(servidor[2])+"."+String(servidor[3])+"\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n";
			client.print(buff);
			buff = ""; */
		}
   String autInterruptor = stringUrl.substring(6, 10);
   String autInterruptorV = stringUrl.substring(10, 11);
	 if(autInterruptor == "int1")
		{
		 if(autInterruptorV == "1")
		 {
		  EEPROM.begin(64);
		  EEPROM.write(MEM_EEPROM_1,1);
		  EEPROM.commit();
		  delay(100);
		  botao1.tipo = byte(EEPROM.read(MEM_EEPROM_1));
		  EEPROM.end();
		  Serial.println("=> TIPO DE SINAL ALTERADO, RELE 1:"+String(botao1.tipo));
		  nContar = 0;
		  }
		 if(autInterruptorV == "0")
		 {
		  EEPROM.begin(64);
		  EEPROM.write(MEM_EEPROM_1,0);
		  EEPROM.commit();
		  delay(100);
		  botao1.tipo = byte(EEPROM.read(MEM_EEPROM_1));
		  EEPROM.end();
		  Serial.println("=> TIPO DE SINAL ALTERADO, RELE 1:"+String(botao1.tipo));
		  nContar = 0;
		}
	}
    if(autInterruptor == "int2")
    {
     if(autInterruptorV == "1")
     {
      EEPROM.begin(64);
      EEPROM.write(MEM_EEPROM_2,1);
      EEPROM.commit();
      delay(100);
      botao2.tipo = byte(EEPROM.read(MEM_EEPROM_2));
      EEPROM.end();
      Serial.println("=> TIPO DE SINAL ALTERADO, RELE 2:"+String(botao2.tipo));
      nContar = 0;
      }
     if(autInterruptorV == "0"){
      EEPROM.begin(64);
      EEPROM.write(MEM_EEPROM_2,0);
      EEPROM.commit();
      delay(100);
      botao2.tipo = byte(EEPROM.read(MEM_EEPROM_2));
      EEPROM.end();
      Serial.println("=> TIPO DE SINAL ALTERADO, RELE 2:"+String(botao2.tipo));
      nContar = 0;
     }
    }
    if(autInterruptor == "int3")
    {
     if(autInterruptorV == "1")
     {
      EEPROM.begin(64);
      EEPROM.write(MEM_EEPROM_3,1);
      EEPROM.commit();
      delay(100);
      botao3.tipo = byte(EEPROM.read(MEM_EEPROM_3));
      EEPROM.end();
      Serial.println("=> TIPO DE SINAL ALTERADO, RELE 3:"+String(botao3.tipo));
      nContar = 0;
      }
     if(autInterruptorV == "0"){
      EEPROM.begin(64);
      EEPROM.write(MEM_EEPROM_3,0);
      EEPROM.commit();
      delay(100);
      botao3.tipo = byte(EEPROM.read(MEM_EEPROM_3));
      EEPROM.end();
      Serial.println("=> TIPO DE SINAL ALTERADO, RELE 3:"+String(botao3.tipo));
      nContar = 0;
    }
  }
  if(autInterruptor == "int4")
    {
     if(autInterruptorV == "1")
     {
      EEPROM.begin(64);
      EEPROM.write(MEM_EEPROM_4,1);
      EEPROM.commit();
      delay(100);
      botao4.tipo = byte(EEPROM.read(MEM_EEPROM_4));
      EEPROM.end();
      Serial.println("=> TIPO DE SINAL ALTERADO, RELE 4:"+String(botao4.tipo));
      nContar = 0;
      }
     if(autInterruptorV == "0"){
      EEPROM.begin(64);
      EEPROM.write(MEM_EEPROM_4,0);
      EEPROM.commit();
      delay(100);
      botao4.tipo = byte(EEPROM.read(MEM_EEPROM_4));
      EEPROM.end();
      Serial.println("=> TIPO DE SINAL ALTERADO, RELE 4:"+String(botao4.tipo));
      nContar = 0;
    }
	} 
	if(requisicao == "00000")//REINCIAR CENTRAL
		{
			ESP.restart();
			//Gravando log de erro na central.
			if (nivel_log >= 1) gravarArquivo( hora+" - Central reiniciada pela pagina WEB...","log.txt");
		}
	if(requisicao == "00010")//CALIBRAR SENSOR MQ2
		{
			calibrarSensor();
		}
	String codidoExec = stringUrl.substring(10,15);
	int valorMQ_Novo = stringUrl.substring(22, 24).toInt();
	if(codidoExec == "00011")
		{
			Serial.println("=> ALTERANDO VALOR DE LEITURA MQ2: "+String(valorMQ_Novo));
			EEPROM.begin(64);
			EEPROM.write(MEM_EEPROM_MQ2,byte(valorMQ_Novo));
			EEPROM.commit();
			LIMITE_MQ2 = byte(EEPROM.read(MEM_EEPROM_MQ2));
			EEPROM.end();
			
		}
	if(requisicao == "00013")//Apagar arquivo de log da cental.
		{
			deletarArquivo("log.txt");
		}
	} 
	//---------------------------------------
	//    PAGINA WEB BACKUP ACESSO DIRETO
	//---------------------------------------

	String buf;
	buf += "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
	buf += "<!DOCTYPE html>";
	buf += "<html lang=\"pt-br\">";
	buf += "<head>";
	buf += "<meta charset=\"utf-8\">";
	buf += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">";
	buf += "<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\">";
	buf += "<title>Central Automação</title></head>";
	buf +="<body>";
	buf +="<div class=\"container shadow-lg p-3 mb-5 bg-white rounded\">";
	buf +="	<ul class=\"nav nav-pills mb-3\" id=\"pills-tab\" role=\"tablist\">";
	buf +="	<li class=\"nav-item\"><a class=\"nav-link active\" id=\"pills-home-tab\" data-toggle=\"pill\" href=\"#pills-home\" role=\"tab\" aria-controls=\"pills-home\" aria-selected=\"true\">Home</a></li>";
	buf +="	<li class=\"nav-item\"><a class=\"nav-link\" id=\"pills-profile-tab\" data-toggle=\"pill\" href=\"#pills-profile\" role=\"tab\" aria-controls=\"pills-profile\" aria-selected=\"false\">Configuração</a></li>";
	buf +="	<li class=\"nav-item\"><a class=\"nav-link\" id=\"pills-contact-tab\" data-toggle=\"pill\" href=\"#pills-contact\" role=\"tab\" aria-controls=\"pills-contact\" aria-selected=\"false\">Contato</a></li>";
	buf +="	</ul>";
	buf +="	<div class=\"tab-content\" id=\"pills-tabContent\">";
	buf +="	<div class=\"tab-pane fade show active\" id=\"pills-home\" role=\"tabpanel\" aria-labelledby=\"pills-home-tab\">";
	buf +="<h4><a href=\"http://"+ipLocalString+"\">CENTRAL -"+ipLocalString+"</a></h4>";
	buf +="	<p style=\"text-align:right\"><span class=\"badge badge-pill badge-primary\">Versão: "+versao+"</span></span></p>";
	buf +="	<table class=\"table table-sm\">";
	buf +="	<thead class=\"thead-light\" ><tr><th>SENSOR</th><th>TIPO</th><th>VALOR</th></tr></thead>";
	buf +="	<tbody><tr><td>DHT11</td><td>Temperatura/Umidade</td><td>" + String(temperatura) + "Cº / " + String(umidade) + "%</td></tr>";
	buf +="	<tr><td>MQ2</td><td>Gás</td><td>" + GLP + " PPM / "+String(sensorMq2) + "</td></tr>";
	buf +="	<tr><td>MQ2</td><td>Fumaça</td><td>" + FUMACA + " PPM / "+String(sensorMq2) + "</td></tr>";
	buf +="	<tr><td></td><td></td><td></td></tr></tbody>";
	buf +="	</table>";
	buf +="	<div style=\"\"><p>";
	if (botao1.estado == true) {
	buf +="	<a href=\"?porta=" + String(botao1.rele) + "&acao=desliga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao1.rele) + " Botão:" + botao1.entrada + "\"><button type=\"button\"  class=\"btn btn-success\">Lampada<br>1</button></a>";
	} else {
		buf +="	<a href=\"?porta=" + String(botao1.rele) + "&acao=liga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao1.rele) + " Botão:" + botao1.entrada + "\"><button type=\"button\"  class=\"btn btn-danger\">Lampada<br>1</button></a>";
	}
	if (botao2.estado == true) {
	buf +="	<a href=\"?porta=" + String(botao2.rele) + "&acao=desliga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao2.rele) + " Botão:" + botao2.entrada + "\"><button type=\"button\"  class=\"btn btn-success\">Lampada<br>2</button></a>";
	} else {
		buf +="	<a href=\"?porta=" + String(botao2.rele) + "&acao=liga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao2.rele) + " Botão:" + botao2.entrada + "\"><button type=\"button\"  class=\"btn btn-danger\">Lampada<br>2</button></a>";
	}
	if (botao3.estado == true) {
	buf +="	<a href=\"?porta=" + String(botao3.rele) + "&acao=desliga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao3.rele) + " Botão:" + botao3.entrada + "\"><button type=\"button\"  class=\"btn btn-success\">Lampada<br>3</button></a>";
	} else {
		buf +="	<a href=\"?porta=" + String(botao3.rele) + "&acao=liga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao3.rele) + " Botão:" + botao3.entrada + "\"><button type=\"button\"  class=\"btn btn-danger\">Lampada<br>3</button></a>";
	}
	if (botao4.estado == true) {
	buf +="	<a href=\"?porta=" + String(botao4.rele) + "&acao=desliga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao4.rele) + " Botão:" + botao4.entrada + "\"><button type=\"button\"  class=\"btn btn-success\">Lampada<br>4</button></a>";
	} else {
		buf +="	<a href=\"?porta=" + String(botao4.rele) + "&acao=liga&central=" + ipLocalString + "\" title=\"Porta:" + String(botao4.rele) + " Botão:" + botao4.entrada + "\"><button type=\"button\"  class=\"btn btn-danger\">Lampada<br>4</button></a>";
	}

	buf +="	</p></div></div>";
	buf +=" <div class=\"tab-pane fade\" id=\"pills-profile\" role=\"tabpanel\" aria-labelledby=\"pills-profile-tab\">";
	
	buf +=" <h4>Configurações De Interruptores</h4><p>";
	if(botao1.tipo == 0){
		buf +="<a href=\?int11\"><button type=\"button\"  class=\"btn btn-dark\">Interruptor 1<br>Sinal -</button></a>";
	}else{
		buf +=" <a href=\"?int10\"><button type=\"button\"  class=\"btn btn-light\">Interruptor 1<br>Sinal +</button></a>";
	}		
	if(botao2.tipo == 0){
		buf +=" <a href=\"?int21\"><button type=\"button\"  class=\"btn btn-dark\">Interruptor 2<br>Sinal -</button></a>";
	}else{
		buf +=" <a href=\"?int20\"><button type=\"button\"  class=\"btn btn-light\">Interruptor 2<br>Sinal +</button></a>";
	}
	if(botao3.tipo == 0){
		buf +=" <a href=\"?int31\"><button type=\"button\"  class=\"btn btn-dark\">Interruptor 3<br>Sinal -</button></a>";
	}else{
		buf +=" <a href=\"?int30\"><button type=\"button\"  class=\"btn btn-light\">Interruptor 3<br>Sinal +</button></a>";
	}
	if(botao4.tipo == 0){
		buf +=" <a href=\"?int41\"><button type=\"button\"  class=\"btn btn-dark\">Interruptor 4<br>Sinal -</button></a>";
	}else{
		buf +=" <a href=\?int40\"><button type=\"button\"  class=\"btn btn-light\">Interruptor 4<br>Sinal +</button></a>";
	}
	/* buf +="<p>";
  buf +="<a  data-toggle=\"collapse\" href=\"#collapseExample\" role=\"button\" aria-expanded=\"false\" aria-controls=\"collapseExample\">";
  buf +="<img src=\"data:image/svg+xml;utf8;base64,PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0iaXNvLTg4NTktMSI/Pgo8IS0tIEdlbmVyYXRvcjogQWRvYmUgSWxsdXN0cmF0b3IgMTYuMC4wLCBTVkcgRXhwb3J0IFBsdWctSW4gLiBTVkcgVmVyc2lvbjogNi4wMCBCdWlsZCAwKSAgLS0+CjwhRE9DVFlQRSBzdmcgUFVCTElDICItLy9XM0MvL0RURCBTVkcgMS4xLy9FTiIgImh0dHA6Ly93d3cudzMub3JnL0dyYXBoaWNzL1NWRy8xLjEvRFREL3N2ZzExLmR0ZCI+CjxzdmcgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIiB4bWxuczp4bGluaz0iaHR0cDovL3d3dy53My5vcmcvMTk5OS94bGluayIgdmVyc2lvbj0iMS4xIiBpZD0iQ2FwYV8xIiB4PSIwcHgiIHk9IjBweCIgd2lkdGg9IjE2cHgiIGhlaWdodD0iMTZweCIgdmlld0JveD0iMCAwIDUxMCA1MTAiIHN0eWxlPSJlbmFibGUtYmFja2dyb3VuZDpuZXcgMCAwIDUxMCA1MTA7IiB4bWw6c3BhY2U9InByZXNlcnZlIj4KPGc+Cgk8ZyBpZD0iYWRkLWNpcmNsZS1vdXRsaW5lIj4KCQk8cGF0aCBkPSJNMjgwLjUsMTI3LjVoLTUxdjEwMmgtMTAydjUxaDEwMnYxMDJoNTF2LTEwMmgxMDJ2LTUxaC0xMDJWMTI3LjV6IE0yNTUsMEMxMTQuNzUsMCwwLDExNC43NSwwLDI1NXMxMTQuNzUsMjU1LDI1NSwyNTUgICAgczI1NS0xMTQuNzUsMjU1LTI1NVMzOTUuMjUsMCwyNTUsMHogTTI1NSw0NTljLTExMi4yLDAtMjA0LTkxLjgtMjA0LTIwNFMxNDIuOCw1MSwyNTUsNTFzMjA0LDkxLjgsMjA0LDIwNFMzNjcuMiw0NTksMjU1LDQ1OXoiIGZpbGw9IiMwMDZERjAiLz4KCTwvZz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8Zz4KPC9nPgo8L3N2Zz4K\" /></a>";
  buf +="</p>";
  buf +="<div class=\"collapse\" id=\"collapseExample\">";
  buf +="<div class=\"card card-body\">";
  buf +="<form class=\"form-group \">";
  buf +="<h4>Alterar Identificação dos Interruptores</h4>";
  buf +="<div class=\"form-group col-md-2\">";
  
  buf +="<input type=\"hidden\" name=\"cod\" value=\"00013\">";
  buf +="<input class=\"form-control form-control-sm\" type=\"text\" name=\"botao_1\" value=\"Interruptor 1\" required>";
  buf +="</div>";
  buf +="<div class=\"form-group col-md-2\">";
  buf +="<input class=\"form-control form-control-sm\" type=\"text\" name=\"botao_2\" value=\"Interruptor 2\" required>";
  buf +="</div>";
  buf +="<div class=\"form-group col-md-2\">";
  buf +="<input class=\"form-control form-control-sm\" type=\"text\" name=\"botao_3\" value=\"Interruptor 3\" required>";
  buf +="</div>";
  buf +="<div class=\"form-group col-md-2\">";
  buf +="<input class=\"form-control form-control-sm\" type=\"text\" name=\"botao_4\" value=\"Interruptor 4\" required>";
  buf +="</div>";
  buf +="<div class=\"form-group col-md-2\">";
  buf +="<button type=\"submit\" class=\"btn btn-primary\">Alterar</button>";
  buf +="</div>";
  buf +="</div>";
  buf +="</form>";
  buf +="</div>";
  buf +="</div>"; */
	buf +="<\p><br><h4>Configurações de Sensores</h4>";
	buf +="<form class=\"form-group\" method=\"get\"><input type=\"hidden\" name=\"cod\" value=\"00011\">Valor Limite Sensor Gás: <input style=\"width:80px\" type=\"text\" placeholder=\"\" name=\"valor\" value=\""+String(LIMITE_MQ2)+"\"> <input class=\"btn btn-info\" type=\"submit\" value=\"Alterar\"><a href=\"?00010\"><button class=\"btn btn-warning\" type=\"button\"  >Recalibrar</button></a></form>";
	buf +="<br><h4>Parâmetros Gerais</h4>";
	buf +="<form class=\"form-group\" action=\"\?00012\"><table>";
	buf +="<tr><td>Servidor: </td><td><input style=\"width:200px\" type=\"text\" placeholder=\"\" name=\"servidor\" value=\"\"></td></tr>";
	buf +="<tr><td>Porta: </td><td><input style=\"width:200px\" type=\"text\" placeholder=\"\" name=\"porta\" value=\"\"></td></tr>/";
	buf +="<tr><td>Interruptor 1: </td><td><input style=\"width:200px\" type=\"text\" placeholder=\"\" name=\"int_1\" value=\"\"></td></tr>";
	buf +="<tr><td>Interruptor 2: </td><td><input style=\"width:200px\" type=\"text\" placeholder=\"\" name=\"int_2\" value=\"\"></td></tr>";
	buf +="<tr><td>Interruptor 3:</td><td><input style=\"width:200px\" type=\"text\" placeholder=\"\" name=\"int_3\" value=\"\"></td></tr>";
	buf +="<tr><td>Interruptor 4: </td><td><input style=\"width:200px\" type=\"text\" placeholder=\"\" name=\"int_4\" value=\"\"></td></tr>";
	buf +="<tr><td></td><td><input class=\"btn btn-info\" type=\"submit\" value=\"Salvar\"></td></tr>";					
	buf +="<table></form>";
	buf +="<br><p><a href=\"?00000\"><button type=\"button\" class=\"btn btn-danger\">Reiniciar Central</button></a></p></div>";
	buf +="<div class=\"tab-pane fade\" id=\"pills-contact\" role=\"tabpanel\" aria-labelledby=\"pills-contact-tab\">...</div></div></div>";
	buf +="<p><a class=\"btn btn-primary\" data-toggle=\"collapse\" href=\"#collapseExample\" role=\"button\" aria-expanded=\"false\" aria-controls=\"collapseExample\">Log da Central</a></p>";
	buf +="<div class=\"collapse\" id=\"collapseExample\">";
	lerArquivo();
	buf +="</div>";
	buf +="</div>";
	//buf +="<!-- JavaScript (Opcional) --><!-- jQuery primeiro, depois Popper.js, depois Bootstrap JS -->";
	buf +="<script src=\"https://code.jquery.com/jquery-3.3.1.slim.min.js\" integrity=\"sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo\" crossorigin=\"anonymous\"></script>";
	buf +="<script src=\"https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.3/umd/popper.min.js\" integrity=\"sha384-ZMP7rVo3mIykV+2+9J3UJ46jBk0WLaUAdn689aCwoqbBJiSnjAK/l8WvCWPIPm49\" crossorigin=\"anonymous\"></script>";
	buf +="<script src=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/js/bootstrap.min.js\" integrity=\"sha384-ChfqqxuZUCnJSK3+MXmPNIyE6ZbWh2IMqE241rYiqJxyMiZ6OW/JmZQ5stwEULTy\" crossorigin=\"anonymous\"></script>";
	buf +="</body></html>";
	client.print(buf);
	client.flush();
	//client.stop();
	
	//---------------------------------------
	//    ROTINA DO SENSOR MQ-2
	//---------------------------------------
	if(millis() >= timeMq2+(timeMq2Param * 1)){
		sensorMq2 = analogRead(PIN_MQ2);
		Serial.println("=> VALOR ANALOGICO: " + String(sensorMq2));
		if(sensorMq2 < 2000){
			GLP = String(getQuantidadeGasMQ(leitura_MQ2(PIN_MQ2)/Ro,GAS_LPG) );
			FUMACA = String(getQuantidadeGasMQ(leitura_MQ2(PIN_MQ2)/Ro,SMOKE));
			Serial.print("=> GLP:"  +GLP+"ppm     " ); 
			Serial.print("CO:"   + String(getQuantidadeGasMQ(leitura_MQ2(PIN_MQ2)/Ro,GAS_CO)  )+"ppm    " ); 
			Serial.print("FUMAÇA:"+FUMACA+"ppm    "); 
			contarParaGravar1++;
			Serial.print("\n Leitura numero: ");
			Serial.println(contarParaGravar1, DEC);
			Serial.println("");
		}else{
			Serial.println("ERRO 0103 - Erro na leitura do sensor de Gás ");
			//Gravando log de erro na central.
			if (nivel_log >= 1) gravarArquivo( hora+" - ERRO 0103","log.txt");
			GLP = "0";
		}
		timeMq2 = millis();
		buff = "sensor=mq-2&valor=mq-2;"+String(GLP)+";&central="+String(ipLocalString)+"&p="+String(PIN_MQ2);
		if (GLP >= LIMITE_MQ2) 
		{
			digitalWrite(BUZZER, !digitalRead(BUZZER));
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
	if(millis() >= timeDht+(timeDhtParam)){
		umidade = dht.readHumidity().toInt();
		temperatura = dht.readTemperature().toInt();
		
		if ((temperatura == temperatura.toInt()) || (umidade == umidade.toInt()) )
		{
			timeDht = millis();
			//String temp = String(temperatura);
			//String umid= String(umidade);
			buff="sensor=dht11&valor=dht11;"+String(temperatura)+";"+String(umidade)+";&central="+String(ipLocalString)+"&p="+String(DHTPIN);
			gravarBanco(buff);
		}else{
			Serial.println(" ERRO 0109 - Erro na leitura dos valores do sensor temperatua e umidade");
			//Gravando log de erro na central.
			if (nivel_log >= 1) gravarArquivo( hora+" - ERRO 0109","log.txt");
		}

	}
	//---------------------------------------

	//---------------------------------------
	//     FIM DA FUNÇÃO LOOP
	//---------------------------------------   
}
<div class="card card-body">
