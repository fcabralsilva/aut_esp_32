	//---------------------------------------  
	//    FUNÇÃO PARA ACIONAMENTO DE PORTAS 
	//    GPIO
	//---------------------------------------  
	void acionaPorta(int numeroF, String portaF, String acaoF) {
	  Serial.println("Comando:"+String(numeroF)+"/"+acaoF);
		if (acaoF == "liga") {
			digitalWrite(numeroF, HIGH );
			linha = "porta="+String(numeroF)+"&acao=liga&central="+ipLocalString;
			gravarBanco(linha);
			linha = "";
			
		}else if (acaoF == "desl") {
			digitalWrite(numeroF, LOW);
			linha = "porta="+String(numeroF)+"&acao=desliga&central="+ipLocalString;
			gravarBanco(linha);
			
		}else if (acaoF == "puls"){
			linha = "porta="+String(numeroF)+"&acao=pulso&central="+ipLocalString;
			gravarBanco(linha);
			digitalWrite(numeroF, HIGH);
			delay(100);
			digitalWrite(numeroF, LOW);
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
				Serial.print("ERRO 0104 - Servidor WEB ou Banco de Dados Desconectado");
				//Gravando log de erro na central.
				if (nivel_log >= 1) gravarArquivo( hora+" - ERRO 0104 "+retorno,"log.txt");
				retorno = "ERRO_SERVIDOR_DESC";
			}else if(r == 1)
			{
				 Serial.print("Servidor WEB ou Banco de Dados Conectado");
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
			Serial.println("ERRO 0105 - Não foi possivel conectar ao servidor WEB ou banco de dados, reiniciando a central!");
			//Gravando log de erro na central.
			if (nivel_log >= 1) gravarArquivo( hora+" - ERRO 0105","log.txt");
			REINICIO_CENTRAL = EEPROM.read(MEM_EEPROM_C);
			if(REINICIO_CENTRAL < 10)
			{
				//ESP.restart();
			}
			//REINICIO_CENTRAL = byte(0);
			//EEPROM.write(MEM_EEPROM_C,REINICIO_CENTRAL );
			//EEPROM.commit();
		}
		Serial.println("Servidor Banco de dados: "+String(servidor));
		if ((client.connect(servidor, portaServidor) == true) && (teste_conexao() == "SERVIDOR_CONECT")) 
			{
				//if (client.connect(servidor, 80)) {
				Serial.println("Enviando dados para o bando de dados:..."+buffer);
				client.println("GET /web/gravar.php?"+buffer);
				client.println();
				buffer = "";
			} else {
				Serial.println("ERRO 0104 - Servidor WEB ou Banco de Dados Desconectado...:"+buffer);
				//Gravando log de erro na central.
				if (nivel_log >= 1) gravarArquivo( hora+" - ERRO 0104","log.txt");
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
			digitalWrite(BUZZER, HIGH);
		}else{
			digitalWrite(BUZZER, LOW);
		}
	}
	//---------------------------------------  


	//---------------------------------------  
	//    FUNÇOES DO MQ / SENSOR DE GAS
	//---------------------------------------  
	void calibrarSensor()
	{
		//CALIBRACAO INCIAL DO SENSOR DE GAS
		Serial.print("\n Caligrando sensor de gás... ");                
		Ro = MQCalibration(PIN_MQ2);                                      
		Serial.println("\n Calibrado com sucesso- VALOR 'Ro' = "+String(Ro)+" kohm"); 
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
	//      FUÇÃO PARA SEPARAR STRINGS 
	//  
	//---------------------------------------  

	String InArray(String S,String T, int P){
		String A = S; 
		String TIPO = T;
		int POSICAO = P, CONTAR = 1;
		int POSICAO_FIM = 0;
		int n = 0;
		int POSICOES_ARRAY[5];
		int p = 0;
		String COMANDO[5];
		while(CONTAR == 1){
			p = A.indexOf(TIPO,POSICAO+1);
			if(p >= 0){
				POSICAO_FIM = p;
				//Serial.println(String(p));
				POSICOES_ARRAY[n] = p;
				n++;
			}else{
				CONTAR = 0;
			}
			delay(10);
		}
		COMANDO[0] = A.substring(0,POSICOES_ARRAY[0]);
		COMANDO[1] = A.substring(POSICOES_ARRAY[0]+1,POSICOES_ARRAY[1]);
		COMANDO[2] = A.substring(POSICOES_ARRAY[1]+1,POSICOES_ARRAY[2]);
		COMANDO[3] = A.substring(POSICOES_ARRAY[2]+1,POSICOES_ARRAY[3]);
		COMANDO[4] = A.substring(POSICOES_ARRAY[3]+1,POSICOES_ARRAY[4]);
		if(POSICAO == 0){
			Serial.println("Posicao 1 ="+String(COMANDO[0]));
			return(COMANDO[0]);
		}else if(POSICAO == 1){
			Serial.println("Posicao 2 ="+String(COMANDO[1]));
			return(COMANDO[1]);
		}else if(POSICAO == 2){
			Serial.println("Posicao 3 ="+String(COMANDO[2]));
			return(COMANDO[2]);
		}else if(POSICAO == 3){
			Serial.println("Posicao 4 ="+String(COMANDO[3]));
			return(COMANDO[3]);
		}else if(POSICAO == 4){
			Serial.println("Posicao 5 ="+String(COMANDO[4]));
			return(COMANDO[4]);
		}else{
			return("NULL");
		}
	}
	//---------------------------------------  


	//---------------------------------------
	//    FUNÇÃO DE MANIPULAÇÃO DE ARQUIVOS  
	//---------------------------------------  
	void formatFS(){
		SPIFFS.format();
	}

	void criarArquivo(){
		openFS();
		File wFile;
		//Cria o arquivo se ele não existir
		if(SPIFFS.exists("/log.txt")){
			Serial.println("Arquivo log.txt ja existe!");
		} else {
			Serial.println("Criando o arquivo log.txt...");
			wFile = SPIFFS.open("/log.txt","w+");
			//Verifica a criação do arquivo
			if(!wFile){
				Serial.println(" ERRO 0108 - Erro ao criar arquivo log.txt!");
				//Gravando log de erro na central.
				if (nivel_log >= 1) gravarArquivo( hora+" - ERRO 0108","log.txt");
			} else {
				Serial.println("Arquivo log.txt criado com sucesso!");
			}
		}
		if(SPIFFS.exists("/param.txt")){
			Serial.println("Arquivo 'param.txt' ja existe!");
		} else {
			Serial.println("Criando o arquivo 'param.txt'...");
			wFile = SPIFFS.open("/param.txt","w+");
			//Verifica a criação do arquivo
			if(!wFile){
				Serial.println(" ERRO 0109 - Erro ao criar arquivo 'param.txt' !");
				//Gravando log de erro na central.
				if (nivel_log >= 1) gravarArquivo( hora+" - ERRO 0109","log.txt");
			} else {
				Serial.println("Arquivo 'param.txt' criado com sucesso!");
			}
		}
		wFile.close();
		closeFS();
	}
	void deletarArquivo(String arquivo) {
		//Remove o arquivo
		if(SPIFFS.remove("/"+arquivo)){
			Serial.println(" ERRO 0105 - Erro ao remover arquivo "+arquivo);
			//Gravando log de erro na central.
			if (nivel_log >= 1) gravarArquivo( hora+" - ERRO 0105","log.txt");
		} else {
			Serial.println(" Arquivo "+arquivo+" removido com sucesso!");
		}
	}
	void gravarArquivo(String msg, String arq) {
		openFS();
		//Abre o arquivo para adição (append)
		//Inclue sempre a escrita na ultima linha do arquivo
		File arquivo = SPIFFS.open("/"+arq,"a+");
		if(!arquivo){
				Serial.println(" ERRO 0106 - Erro ao abrir arquivo "+arq+" no sistema de arquivo interno da central!");
				//Gravando log de erro na central.
				if (nivel_log >= 1) gravarArquivo( hora+" - ERRO 0106","log.txt");
			} else {
				arquivo.println(msg);
				Serial.println(" Dados gravado no arquivo "+arq+" corretamente");
			}
		arquivo.close();
		closeFS();
		/* if(arq == "log.txt")
		{
			File logg = SPIFFS.open("/log.txt","a+");
			if(!logg){
				Serial.println("ERRO 0106 - Erro ao abrir arquivo no sistema de arquivo interno da central!");
			} else {
				logg.println("Log: " + msg);
				Serial.println(msg);
			}
			logg.close();
		}
		
		if(arq == "param.txt")
		{
			File param1 = SPIFFS.open("/param.txt","a+");
			if(!param1){
				Serial.println("Erro ao abrir arquivo!");
			} else {
				param1.println(msg);
				Serial.println(msg);
			}
			param1.close();
		} */
	}
	void lerArquivo() {
		//Faz a leitura do arquivo
		String buff;
		File ARQUIVO = SPIFFS.open("/log.txt","r");
		Serial.println(" Reading file...");
		while(ARQUIVO.available()) {
			String line = ARQUIVO.readStringUntil('\n');
			buff += line;
			buff += "<br />";
		}
		ARQUIVO.close();
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
		if(!SPIFFS.begin()){
			Serial.println(" ERRO 0107 - Erro ao abrir sistema de arquivo interno da central!"");
			//Gravando log de erro na central.
			if (nivel_log >= 1) gravarArquivo( hora+" - ERRO 0107","log.txt");
		} else {
			Serial.println(" Sistema de arquivos aberto com sucesso!");
		}
	}
	//---------------------------------------  

	//callback que indica que o ESP entrou no modo AP
	void configModeCallback (WiFiManager *myWiFiManager) {  
	//  Serial.println("Entered config mode");
	  Serial.println("Entrou no modo de configuração");
	  Serial.println(WiFi.softAPIP()); //imprime o IP do AP
	  Serial.println(myWiFiManager->getConfigPortalSSID()); //imprime o SSID criado da rede

	}

	//callback que indica que salvamos uma nova rede para se conectar (modo estação)
	void saveConfigCallback () {
	//  Serial.println("Should save config");
	  Serial.println("Configuração salva");
	  Serial.println(WiFi.softAPIP()); //imprime o IP do AP
	}

