void dataHoraNTP()
{
	DateTime now = rtc.now();
	String anoRTC = String(now.year());
	String mesRTC = String(now.month());
	String diaRTC = String(now.day());
	char *diaSemanaRTC = daysOfTheWeek[now.dayOfTheWeek()];
	int hora_RTC = now.hour();
	int minutoRTC = now.minute();
	int segundosRTC = now.second();
	hora_ntp   = timeClient.getFormattedTime();
	hora_rtc = String(hora_RTC) + ":" + String(minutoRTC) + ":" + String(segundosRTC);
}

void portaIO(int entrada, int rele, const char* tipo,const char* modelo,char contador, boolean estado){
	String s_tipo_1 = String(tipo);
	String s_modelo_1 = String(modelo);
	if (s_modelo_1 == "pulso")
	{

		if (digitalRead(entrada) == s_tipo_1.toInt())
		{
			if (nContar == 0)Serial.println(" Entrada "+String(entrada)+" - Modo pulso... ");
			while ((digitalRead(entrada) == s_tipo_1.toInt()) && (nContar <= 100) )
			{
				if (millis() >= tempo + paramTempo)
				{
					contador++;
					nContar++;
					Serial.print(contador, DEC);
					tempo = millis();
				}
			}
		}
	} else if (s_modelo_1 == "interruptor")
	{

		estado_atual = digitalRead(entrada);
		if (estado_atual != estado_antes )
		{
			if (nContar == 0)Serial.println(" Entrada "+String(entrada)+" - Modo interruptor... ");
			estado_antes = estado_atual;
			contador = 3;
			Serial.print(contador, DEC);
		}
	}
	if ((contador >= 2) && (contador <= 9))
	{
		if (nContar >= 100)
		{
			
		if(n == 0)
		{
			for(int i = 0; i <=0 ;i++ )
			{
				String ERRO_ENTRADA = hora_rtc + " - ERRO 0107 - Interruptor Porta IN: "+String(rele)+" Porta OUT: "+String(entrada)+" esta com erro de execução, deve usar a pagina para reiniciar";
				//Gravando log de erro na central.
				if ((nivel_log >= 1) || (logtxt == "sim")) gravarArquivo( ERRO_ENTRADA, "log.txt");
				n = 1;
			}
		}
	} else
		{
			String ERRO_ENTRADA = "0";
			nContar = 0;
			if (estado == false) {
				Serial.println(" Ligando Porta "+String(entrada)+" : " + String(rele));
				estado = true;
				contador = 0;
				acionaPorta(rele, "", "liga");
			} else {
				Serial.println(" Desligar Porta "+String(entrada)+" : " + String(rele));
				acionaPorta(rele, "", "desl");
				estado = false;
				contador = 0;
			}
		}
	}

}

const char* abreJson(const char* json, const char* etiqueta  )
{
  DynamicJsonDocument doc;
  const char* servidor;
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    Serial.print(F(" ERRO 0101 - Falha ao desenraizar o arquivo json: "));
    Serial.println(error.c_str());
  
  }
  JsonObject root = doc.as<JsonObject>();
  servidor       = root[etiqueta];
  root.end();
  return servidor;
}

void gravaLog(String mensagem, String permissao, int nivel)
{
	Serial.println(mensagem);
	for(int i = 0; i <=0 ;i++ )
	{
		if (logtxt == "sim")
		{
			if( String(nivel) >= String(nivelLog))
			{
				gravarArquivo(mensagem, "log.txt");
			} 
		}
	}
}

String selectedHTNL(const char* tipo, String comp )
{
	String select;
	String s_tipo = String(tipo);
	String s_comp = String(comp);
	if (s_tipo == comp) {
		return select = "selected";
	} else {
		return select = "";
	}
}

void ajustaHora( int ano, int mes, int dia, int hora, int minuto ){
    Serial.println(" Corrigindo data e hora...");
    rtc.adjust(DateTime(ano, mes, dia, hora, minuto, 0));
  }
  
String quebraString(String txtMsg,String string)
{ 
	unsigned int tamanho = txtMsg.length();
	int inicio_string = string.indexOf(txtMsg)+tamanho+1;
	int final_string = string.indexOf("&",inicio_string);
	String resultado = string.substring(inicio_string, final_string);
	//Serial.println(resultado);
	return resultado;
} 
  
void forceUpdate(void) 
{
	timeClient.forceUpdate();
}

void checkOST()
{
	currentMillis = millis();//Tempo atual em ms
	//Lógica de verificação do tempo
	if (currentMillis - previousMillis > 1000) 
	{
		previousMillis = currentMillis;    // Salva o tempo atual
		printf("Time Epoch: %d: ", timeClient.getEpochTime());
		//Serial.println(timeClient.getFormattedTime());
	}
}

//---------------------------------------  
//    FUNÇÃO PARA ACIONAMENTO DE PORTAS 
//    GPIO
//---------------------------------------  
void acionaPorta(int numeroF, String portaF, String acaoF) {
  Serial.println(" Comando:"+String(numeroF)+"/"+acaoF);
	if (acaoF == "liga") {
		digitalWrite(numeroF, HIGH );
		linha = "porta="+String(numeroF)+"&acao=liga&central="+ipLocalString;
		gravarBanco(linha);
		gravaLog(" "+hora_ntp + " - COMANDO: "+linha, logtxt, 4);
		linha = "";
		
	}else if (acaoF == "desl") {
		digitalWrite(numeroF, LOW);
		linha = "porta="+String(numeroF)+"&acao=desliga&central="+ipLocalString;
		gravarBanco(linha);
		gravaLog(" "+hora_ntp + " - COMANDO: "+linha, logtxt, 4);
		linha = "";
		
	}else if (acaoF == "puls"){
		linha = "porta="+String(numeroF)+"&acao=pulso&central="+ipLocalString;
		gravarBanco(linha);
		digitalWrite(numeroF, HIGH);
		delay(1000);
		digitalWrite(numeroF, LOW);
		linha = "";
	}
	
}

String teste_conexao(){
	WiFiClient client = server.available();
 	if(millis() >= time3+time3Param)
	{
		time3 = millis();
		 
		int r = client.connect(servidor, portaServidor);
		if(r == 0)
		{
			gravaLog(" "+hora_ntp + " - ERRO 0104 - Servidor WEB ou Banco de Dados Desconectado", logtxt, 1);
			retorno = "ERRO_SERVIDOR_DESC";
		}else if(r == 1)
		{
			 Serial.println(" Servidor WEB/Banco OK ");
			 retorno = "SERVIDOR_CONECT";
		}
	}
	return retorno;
}

//---------------------------------------  
//    FUNÇÃO PARA GRAVAR NO BANCO
//---------------------------------------  
void gravarBanco (String buffer){
	WiFiClient client = server.available();
	if(WiFi.status() != WL_CONNECTED)
	{
		gravaLog(" "+hora_ntp + " - ERRO 0105 - Não foi possivel conectar ao servidor WEB ou banco de dados, reiniciando a central!", logtxt, 1);
		WiFi.reconnect();
		if(WiFi.status() != WL_CONNECTED){
			Serial.println(" ");
			gravaLog(" "+hora_ntp + " - Falha ao conectar ao WIFI e atingir o tempo limite, reiniciando a central!", logtxt, 1);
			//ESP.restart();
			delay(1000);
		} 
	}
	Serial.println(" Servidor Banco de dados: "+String(servidor));
	if ((client.connect(servidor, portaServidor) == true) && (teste_conexao() == "SERVIDOR_CONECT")) 
		{
			//if (client.connect(servidor, 80)) {
			Serial.print(" Enviando..."+buffer);
			client.println("GET /web/gravar.php?"+buffer);
			//gravaLog(" "+hora_ntp + " - COMANDO: "+buffer, logtxt, 4);
			client.println();
			buffer = "";
		} else {
			gravaLog(" "+hora_ntp + " - ERRO 0104 - Servidor WEB ou Banco de Dados Desconectado...:", logtxt, 1);
			buffer = "";
		}

	client.flush();
	client.stop();
}
//---------------------------------------  


//---------------------------------------  
//    FUNÇÃO DA SIRENE
//--------------------------------------- 
void sirene(boolean valor){
	if(valor == true){
		 ledcWriteTone(channel, 2000);
     delay(400);
     ledcWriteTone(channel, 1800);
     delay(400);
     ledcWriteTone(channel, 1000);
     delay(300);
     
	}else{
		ledcWriteTone(channel, false);
	}
}
//---------------------------------------  


//---------------------------------------  
//    FUNÇOES DO MQ / SENSOR DE GAS
//---------------------------------------  
void calibrarSensor()
{
	//CALIBRACAO INCIAL DO SENSOR DE GAS
	Serial.print(" Caligrando sensor de gás... ");                
	Ro = MQCalibration(PIN_MQ2);                                      
	Serial.println(" Calibrado com sucesso- VALOR 'Ro' = "+String(Ro)+" kohm"); 
}

float calcularResistencia(int tensao)   //funcao que recebe o tensao (dado cru) e calcula a resistencia efetuada pelo sensor. O sensor e a resistência de carga forma um divisor de tensão. 
{
	return (((float)VRL_VALOR*(1023-tensao)/tensao));
}

float MQCalibration(int mq_pin)   //funcao que calibra o sensor em um ambiente limpo utilizando a resistencia do sensor em ar limpo 9.83
{
	int i;
	float valor=0;

	for (i=0;i<ITERACOES_CALIBRACAO;i++) {    //sao adquiridas diversas amostras e calculada a media para diminuir o efeito de possiveis oscilacoes durante a calibracao
		Serial.print(".");
		valor += calcularResistencia(analogRead(mq_pin));
		delay(500);
		digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));//Faz o LED piscar (inverte o estado).
	}
	valor = valor/ITERACOES_CALIBRACAO;        
	valor = valor/RO_FATOR_AR_LIMPO; //o valor lido dividido pelo R0 do ar limpo resulta no R0 do ambiente
	return valor; 
}
float leitura_MQ2(int mq_pin)
{
	int i;
	float rs=0;
	for (i=0;i<ITERACOES_LEITURA;i++) {
		rs += calcularResistencia(analogRead(mq_pin));
		digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));//Faz o LED piscar (inverte o estado).
		delay(10);
	}
	rs = rs/ITERACOES_LEITURA;
	return rs;  
}
int getQuantidadeGasMQ(float rs_ro, int gas_id)
{
	if ( gas_id == 0 ) {
		return calculaGasPPM(rs_ro,LPGCurve);
	} else if ( gas_id == 1 ) {
		return calculaGasPPM(rs_ro,COCurve);
	} else if ( gas_id == 2 ) {
		return calculaGasPPM(rs_ro,SmokeCurve);
	}    

	return 0;
}
int  calculaGasPPM(float rs_ro, float *pcurve) //Rs/R0 é fornecido para calcular a concentracao em PPM do gas em questao. O calculo eh em potencia de 10 para sair da logaritmica
{
	return (pow(10,( ((log(rs_ro)-pcurve[1])/pcurve[2]) + pcurve[0])));
}
//---------------------------------------  

//---------------------------------------
//    FUNÇÃO DE MANIPULAÇÃO DE ARQUIVOS  
//---------------------------------------  
void formatFS(){
	SPIFFS.format();
}

void criarArquivo(){
	File wFile;
	//Cria o arquivo se ele não existir
	if(SPIFFS.exists("/log.txt")){
		Serial.println(" Arquivo log.txt ja existe!");
	} else {
		Serial.println(" Criando o arquivo log.txt...");
		wFile = SPIFFS.open("/log.txt","w+");
		//Verifica a criação do arquivo
		if(!wFile){
			gravaLog(" "+hora_ntp + " - ERRO 0108 - Erro ao criar arquivo log.txt", logtxt, 1);
		} else {
			Serial.println(" Arquivo log.txt criado com sucesso!");
		}
	}
	if(SPIFFS.exists("/param.txt")){
		Serial.println(" Arquivo 'param.txt' ja existe!");
	} else {
		Serial.println(" Criando o arquivo 'param.txt'...");
		wFile = SPIFFS.open("/param.txt","w+");
		//Verifica a criação do arquivo
		if(!wFile){
			gravaLog(" "+hora_ntp + " - ERRO 0109 - Erro ao criar arquivo 'param.txt'", logtxt, 1);
		} else {
			Serial.println(" Arquivo 'param.txt' criado com sucesso!");
		}
	}
	wFile.close();
}
void deletarArquivo(String arquivo) {
	//Remove o arquivo
	if(!SPIFFS.remove(arquivo)){
		gravaLog(" "+hora_ntp + " - ERRO 0105 - Erro ao remover arquivo "+arquivo, logtxt, 1);
	} else {
		Serial.println(" Arquivo "+arquivo+" removido com sucesso!");
	}
}
void gravarArquivo(String msg, String arq) {
	//Abre o arquivo para adição (append)
	//Inclue sempre a escrita na ultima linha do arquivo
	if(arq == "log.txt")
	{
		File logg = SPIFFS.open("/log.txt","a+");
		int s = logg.size(); // verificar tamanho do arquivo
		if (s >= 20000){
			deletarArquivo("/log.txt");
			criarArquivo(); 
			delay(1000);
			gravaLog(" "+hora_ntp + " - Log foi deletado por estar grande demais! ", logtxt, 1);
		}
		if(!logg){
			//Gravando log de erro na central.
			Serial.println(" ERRO 0108 - Erro ao abrir arquivo "+arq);
		} else {
			logg.println(msg);
			Serial.println(msg);
		}
		logg.close();
	}
	
	if(arq == "param.txt")
	{
		File param1 = SPIFFS.open("/param.txt","a+");
		if(!param1){
			gravaLog(" "+hora_ntp + " - ERRO 0106 - Erro ao abrir arquivo "+arq, logtxt, 1);
		} else {
			param1.println(msg);
			Serial.println(" Gravando Paramentos: "+msg);
		}
		param1.close();
	}
}
String lerArquivo() {
	//Faz a leitura do arquivo
	String buff;
	File ARQUIVO = SPIFFS.open("/log.txt","r");
	int tamanhoLog = ARQUIVO.size(); // verificar tamanho do arquivo
    buff = "Tamanho log.txt: "+String(tamanhoLog)+"<br />"; 
	while(ARQUIVO.available()) {
		String line = ARQUIVO.readStringUntil('\n');
		buff += line+"<br />";
	}
	ARQUIVO.close();
	return buff;
}
String lerArquivoParam(void) {
	String buff;
  File ARQUIVO = SPIFFS.open("/param.txt","r");
  //Serial.println("Lendo Parametros da central...");
  while(ARQUIVO.available()) {
    String line = ARQUIVO.readStringUntil('\n');
    buff += line;
  }
  ARQUIVO.close();
  return buff;
}
void closeFS(){
	SPIFFS.end();
}
void openFS(){
	//Abre o sistema de arquivos
  SPIFFS.begin();
	if(!SPIFFS.begin()){
		gravaLog(" "+hora_ntp + " - ERRO 0107 - Erro ao abrir sistema de arquivo interno da central! ", logtxt, 1);
		} else {
		Serial.println(" Sistema de arquivos aberto com sucesso!");
	}
}
//---------------------------------------  

//callback que indica que o ESP entrou no modo AP
void configModeCallback (WiFiManager *myWiFiManager) {  
//  Serial.println("Entered config mode");
  Serial.println(" Entrou no modo de configuração ");
  Serial.println(WiFi.softAPIP()); //imprime o IP do AP
  Serial.println(myWiFiManager->getConfigPortalSSID()); //imprime o SSID criado da rede

}

//callback que indica que salvamos uma nova rede para se conectar (modo estação)
void saveConfigCallback () {
//  Serial.println("Should save config");
  Serial.println(" Configuração salva ");
  Serial.println(WiFi.softAPIP()); //imprime o IP do AP
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listando Diretórios: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- Falha ao abrir diretório");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - Diretório não encontrado");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  Arquivo: ");
            Serial.print(file.name());
            Serial.print("\tTamanho: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}
