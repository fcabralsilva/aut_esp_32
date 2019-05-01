<p><strong>PORTAS ENTRADA SA&Iacute;DA</strong></p>
<p>entrada 1 = 32, rele 1 = 33<br />entrada 2 = 25, rele 2 = 18<br />entrada 3 = 14, rele 3 = 27<br />entrada 4 = 12, rele 4 = 13<br />entrada 5 = <span class="pl-c1">16</span>, rele 5 = <span class="pl-c1">18</span></p>
<p><strong><span class="pl-c1">LED'S PARA MONITORAMENTO</span></strong></p>
<p><span class="pl-c1">LED_AZUL&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; 2<br />LED_VERDE&nbsp; &nbsp; &nbsp; &nbsp; &nbsp;4<br />LED_VERMELHO 16</span></p>
<p><strong><span class="pl-c1">SENSORES</span></strong></p>
<p><span class="pl-c1">BUZZER&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; 5<br />PIN_MQ2&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;34<br />DHTPIN&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;19</span></p>
<p><strong>REINCIAR CENTRAL POR COMANDA HTTP</strong>&nbsp;</p>
<p><a href="HTTP://IP_HOST/?00000">HTTP://IP_HOST/?00000</a></p>
<p><strong>EXEMPLO NA CHAMADA WEB DESLIGAR LAMPADA</strong>&nbsp;</p>
<p><a href="HTTP://IP_HOST/?porta=NN&amp;acao=(liga%20ou&nbsp;desligar)&amp;central=IP_HOST">HTTP://IP_HOST/?porta=NN&amp;acao=(liga ou&nbsp;desligar)&amp;central=IP_HOST</a></p>
<p><strong>CALIBRAR SENSOR MQ2</strong></p>
<p><a href="HTTP://IP_HOST/?00010">HTTP://IP_HOST/?00010</a></p>
<p><strong>GRAVAR VALOR DE LEITURA DO SENSOR DE GAS NA EEPROM</strong>&nbsp;</p>
<p><a href="HTTP://IP_HOST/?00011">HTTP://IP_HOST/?00011</a></p>
<p><strong>APAGAR ARQUIVO DE LOG MANUALMENTE</strong></p>
<p><a href="HTTP://IP_HOST/?00013">HTTP://IP_HOST/?00013</a></p>
<p><strong>APLICAR CONFIGURA&Ccedil;&Otilde;ES MINIMAS PARA FUNCIONAMENTO DA CENTRAL</strong></p>
<p><a href="HTTP://IP_HOST/?00014">HTTP://IP_HOST/?00014</a></p>
<p><strong>DESLIGAR TODOS AS PORTAS OUTPUT DA CENTRAL</strong></p>
<p><a href="HTTP://IP_HOST/?00015">HTTP://IP_HOST/?00015</a></p>
<p><strong>APLICAR AS CONFIGURA&Ccedil;&Otilde;ES AP&Oacute;S SEREM GRAVADAS NA CENTRAL</strong>&nbsp;</p>
<p><a href="HTTP://IP_HOST/?00016">HTTP://IP_HOST/?00016</a></p>
<p><strong>FUN&Ccedil;&Atilde;O DE CONTROLE DO ALARME</strong> -</p>
<p>ATIVAR&nbsp; -&nbsp; <a href="HTTP://IP_HOST/?00117">HTTP://IP_HOST/?00117</a></p>
<p>DESATIVAR&nbsp; - <a href="HTTP://IP_HOST/?00017">HTTP://IP_HOST/?00017</a></p>