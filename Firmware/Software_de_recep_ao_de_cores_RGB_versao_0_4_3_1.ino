/*********************************************************************
**  Device: TSC3200                                                 **
**  File:   EspectrofotometroGME.ino                                **
**                                                                  **
**  Created by GME Wanderson D. Lopes /24 Apr 2016                  **
**                                                                  **
**  Description:                                                    **
**  Incorporacao de calibracao em tempo de execucao, tabela forne-  **
**  cida ao usuario adaptada ao experimento de deteccao de cor de   **
**  amostras para Quimica Analitica.                                **
**                                                                  **
**                                                                  **
**                                                                  **
**  https://wdldev.blogspot.com                                     **
**                                                                  **
*********************************************************************/


#include <TimerOne.h>

#define S0     6
#define S1     5
#define S2     4
#define S3     3
#define OUT    2

int   g_count = 0;      // contador de frequencia
int   g_array[4];       // guarda os valores da frequencia
int   g_flag = 0;       // flag se selecao de do filtro RGB
float g_SF[3];          // Salva o fator de escala RGB
boolean ledstate = HIGH; // Fornece o valor do estado do led 
int amostra = 1; // varivel responsavel por numerar as amostras na saida de dados
// Inicia as portas de comunicacao e frequencia trabalhada
void TSC_Init()
{
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT, INPUT);
  pinMode(10,OUTPUT);
  
  digitalWrite(S0, LOW);  // Frequancia de saida determinada como 2%
  digitalWrite(S1, HIGH); // esses dois pinos sao responsaveis por indicar a escala de frequencia
}
 
// Select the filter color 
void TSC_FilterColor(int Level01, int Level02) // essa funcao eh responsavel por setar os fotodiodos que fornecerao o sinal em OUT
{
  if(Level01 != 0)
    Level01 = HIGH; // se o nevel nao for baixo ou seja algo diferente de baixo, mudar para alto, usado para corrigir incoerencias no codigo
 
  if(Level02 != 0)
    Level02 = HIGH; // se o nevel nao for baixo ou seja algo diferente de baixo, mudar para alto, sado para corrigir incoerencias no codigo
 
  digitalWrite(S2, Level01); 
  digitalWrite(S3, Level02); 
}
 
void TSC_Count()
{
  g_count ++ ;
}
 
void TSC_Callback()
{
  switch(g_flag)
  {
    case 0: 
         TSC_WB(LOW, LOW);              
         break;
    case 1:
         //Serial.print("Frequancia Filtro R="); //Arranjo de diodos com filtro Red
         //Serial.println(g_count);
         g_array[0] = g_count;
         TSC_WB(HIGH, HIGH);            
         break;
    case 2:
         //Serial.print("Frequancia Filtro G="); //Arranjo de diodos com filtro Green
         //Serial.println(g_count);
         g_array[1] = g_count;
         TSC_WB(LOW, HIGH);             //Arranjo de diodos com filtro Blue
         break;
 
    case 3:
         //Serial.print("Frequancia Filtro B=");
         //Serial.println(g_count);
         g_array[2] = g_count;
         TSC_WB(HIGH, LOW);             //Arranjo de diodos com filtro abertos (sem filtro)      
         break;
    case 4:
         g_array[4] = g_count;
         break;
   default:
         g_count = 0;
         break;
  }
}
 
void TSC_WB(int Level0, int Level1)      //White Balance
{
  g_count = 0;
  g_flag ++;
  TSC_FilterColor(Level0, Level1);
  Timer1.setPeriod(1000000);             // set 1s period
}
 
void setup()
{
  TSC_Init();
  Serial.begin(9600);
  Serial.println("Leituras Iniciais");
  Timer1.initialize();             // defaulte is 1s
  Timer1.attachInterrupt(TSC_Callback);  
  attachInterrupt(0, TSC_Count, RISING);  // toda vez que o pino 2 apresentar uma mudanca de sinal de 0 para 1 ocorre uma chamada da funcao TSC_count
  delay(4000);
  Serial.println("Iniciando leitura de detector RGB");
  Serial.println("Aguardando comando para inicio da calibracao");
  digitalWrite(10,HIGH); //  liga o led emissor
  while(!Serial.available()){}
  int le = Serial.read(); // limpa o buffer da Serial
  delay(4000); // delay de 4 segundos para estabilizar a condicao do detector e emissor
  Maxtransmitancia();
}
 
void loop(){
  g_flag = 0;
  char RGB[3]={'R','G','B'};
  while(!Serial.available()){  // Saida de dados
     Serial.print(amostra);
     Serial.print("\t");    
    for(int i=0; i<3; i++){    
     Serial.print(g_array[i]);
     Serial.print("\t"); 
    }
    Serial.print(round(g_array[0] * g_SF[0])); 
    Serial.print("\t");  
    Serial.print(round(g_array[1] * g_SF[1]));
    Serial.print("\t"); 
    Serial.println(round(g_array[2] * g_SF[2]));
    amostra++;
    g_flag = 0;
    delay(4000);
  }
  if(Serial.read() == 'c'){  // Ao enviar C eh executada a calibracao
    Serial.println();
    Serial.println();
    ledstate = HIGH; //muda o marcador para HIGH antes de calibrar
    digitalWrite(10,HIGH); // liga o led emissor
    delay(4000); // delay de 4 segundos para estabilizar a condicao do detector e emissor
    Maxtransmitancia(); // chama a funcao de calibracao
    amostra = 1;  // apos a calibracao retorna o valor para amostras iniciais para a nova curva de calibracao
  }
  else if(ledstate == HIGH){
    digitalWrite(10,LOW);
    ledstate = LOW;
  }
    else{
      digitalWrite(10,HIGH);
      ledstate = HIGH;
      delay(4000);
    }
}

void Maxtransmitancia(){
  g_flag = 0;
  delay(4000);
  g_SF[0] = 255.000/ g_array[0];    //fator de escala R
  g_SF[1] = 255.000/ g_array[1];    //fator de escala G
  g_SF[2] = 255.000/ g_array[2];    //fator de escala B
  
  char RGB[3]={'R','G','B'};
  Serial.println("Inicio da Calibraçao");
  Serial.println();
  Serial.println("Filtro\tNiveis Maximos\tCoeficiente de Calibracao");
  for(int i=0; i<3; i++){
    Serial.print(RGB[i]);
    Serial.print("\t");    
    Serial.print(g_array[i]);
    Serial.print("\t");
    Serial.println(g_SF[i]);
  }
  Serial.println("Fim da calibracao");
  Serial.println();
  Serial.println();
  Serial.println("Amostra\tfreq. R\tfreq. G\tfreq. B\tR Cal.\tR Cal.\tG Cal.\tB Cal."); // imprime o cabecalho
}
