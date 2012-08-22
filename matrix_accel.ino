
/************************************************************************************************************************************************************************************************************/
/* Distribuido bajo Reconocimiento-NoComercial-CompartirIgual 3.0 España (CC BY-NC-SA 3.0)
Esto es un resumen legible por humanos del texto legal (la licencia completa) disponible en los idiomas siguientes: Asturiano Castellano Catalán Euskera Gallego
Advertencia
Usted es libre de:

copiar, distribuir y comunicar públicamente la obra
Remezclar — transformar la obra
Bajo las condiciones siguientes:

Reconocimiento — Debe reconocer los créditos de la obra de la manera especificada por el autor o el licenciador (pero no de una manera que sugiera que tiene su apoyo o apoyan el uso que hace de su obra).
No comercial — No puede utilizar esta obra para fines comerciales.
Compartir bajo la misma licencia — Si altera o transforma esta obra, o genera una obra derivada, sólo puede distribuir la obra generada bajo una licencia idéntica a ésta.
Entendiendo que:

Renuncia — Alguna de estas condiciones puede no aplicarse si se obtiene el permiso del titular de los derechos de autor
Dominio Público — Cuando la obra o alguno de sus elementos se halle en el dominio público según la ley vigente aplicable, esta situación no quedará afectada por la licencia.
Otros derechos — Los derechos siguientes no quedan afectados por la licencia de ninguna manera:
Los derechos derivados de usos legítimos u otras limitaciones reconocidas por ley no se ven afectados por lo anterior.
Los derechos morales del autor;
Derechos que pueden ostentar otras personas sobre la propia obra o su uso, como por ejemplo derechos de imagen o de privacidad.
Aviso — Al reutilizar o distribuir la obra, tiene que dejar bien claro los términos de la licencia de esta obra.*/
/************************************************************************************************************************************************************************************************************/

/****************************************************/
/*   Copyright 2012 A.Prado  a.k.a. droky           */
/*   @radikaldesig  radikaldesig.com                */
/*  Licenciado bajo Creative Commons 3.0            */
/****************************************************/


#include "HT1632.h"  //Libreria de control de matrices de leds
                     //Usando un HT1632 incorporado en la JY-3208
                     //Reloj led 32x8 de DealExtreme.com
//Definiciones de Pines en Placa JY-3208
#define DATA 13
#define WR   12
#define CS   11


// Nuevo objeto matriz de clase HT1632LEDMatrix
HT1632LEDMatrix matrix = HT1632LEDMatrix(DATA, WR, CS);

//Definiciones de pines Acelerometro LIS344H montado en STEVAL-MKIO15V1 
//de STMicroelectronics
#define aX A0  //Accel eje x
#define aY A1  //Accel eje y

//Variables de entorno
const int numeroBolitas = 5; //Numero de particulas
float elasticidad = -0.75;   //coeficiente de elasticidad con las paredes
                             //Es negativo para cambiar el 
                             //sentido del vector velocidad
                             //en el choque
float friccion = 0.35;  //Constante de friccion entre particulas 
int anchoDisplay = 32;  //Ancho matriz
int altoDisplay = 8;    //Alto mtriz
int factorReduccion = 1000;  //Factor de reduccion para suavizar movimientos

const float factorFiltro = 0.95;  //Factor de filtrado del acelerometro
//El factor de filtrado esta incluido en un filtro paso-bajo. Para el caso 
//de un sistema de particulas, es necesario un valor bajo de filtrado, ya
//que al usar un suavizado mayor, desaparecen los desplazamientos de las
//particulas con golpes o vibraciones de corta duraciion y el movimiento
//de estas pierde realismo

double zeroValue[2] = { 0 }; // ZeroG gyroX, gyroY

//Valores en reposo para los ejes x,y medida anteriormente
//De ser necesario, se usara la rutina calibrado_sensor()
//para obtener nuevos valores de posicion en reposo
double LoX = 506.90;
double LoY = 506.90;

//Estructura de las particulas
struct particula {
 double posX;    //Posicion X
 double posY;    //Posicion Y
 float velX;    //Velocidad X
 float velY;    //Velocidad Y
 float masa;    //Masa de la particula
  
};

//Crear particulas
particula bolita[numeroBolitas]={0,0,0,0};


/****************************************************/
/*                                                  */
/*                SetUp                             */
/*                                                  */
/****************************************************/
void setup() {
  analogReference(EXTERNAL); // tension de referencia del ADC=3.3V
  Serial.begin(115200);      //Comunicacion serie a 115200bps
  delay(100);   //Esperamos que el sensor se estabilice

// Llamada a rutina de calibracion en reposo si es necesario calibrar
// calibrado_sensor();

//Valores de calibracion obtenidos anteriormente en ZeroG                       
 zeroValue[0] = 506.90;
 zeroValue[1] = 506.90;  

//Posicionado aleatorio de las particulas con velocidades=0
//Se asigna una masa aleatoria de entre 1<=m<=2
for (int o = 0;o < numeroBolitas; o++){
  bolita[o].posX = random(anchoDisplay*factorReduccion);
  bolita[o].posY = random(altoDisplay*factorReduccion);
  bolita[o].velX = 0;
  bolita[o].velY = 0;
  bolita[o].masa = random(10,20)/10;
}

//Definiciones de la Matriz
  matrix.begin(HT1632_COMMON_8NMOS);  //Inicializar matriz
  delay(500);    //Tiempo de estabilizacion
  matrix.clearScreen();  //Borrado de matriz
  
}

/****************************************************/
/*                                                  */
/*                Bucle principal                   */
/*                                                  */
/****************************************************/
void loop() {
  lee_acelerometro();  // Lee acelerometro
  //print_datos();     // Solo para debug
  colision_bolas();    // Calculo de colisiones
  mueve_bolas();       // Movimiento de las particulas
  pon_bolas();         // Poner particulas en el Display
  delay(4);    // Retardo dependiente del numero de particulas 
               // en la matriz para controlar el refresco
}

/****************************************************/
/*                                                  */
/*  Leer acelerometro y filtrar ruido con FPB       */
/*                                                  */
/****************************************************/
void lee_acelerometro(){
  //Leemos las entradas analogicas y le restamos el valor Zero  
  double accXval = (double)analogRead(aX)-zeroValue[0];
  double accYval = (double)analogRead(aY)-zeroValue[1];
  //Filtro paso Bajo
  LoX = accXval * factorFiltro + (LoX * (1.0 - factorFiltro));
  LoY = accYval * factorFiltro + (LoY * (1.0 - factorFiltro));
}

/****************************************************/
/*                                                  */
/*   Funcion de calibrado (lectura valores ZeroG    */
/*                                                  */
/****************************************************/
void calibrado_sensor(){
  // Calibracion de los sensores en posicion horizontal
  // Hacemos 100 lecturas y una media matematica :)
  for (uint8_t i = 0; i < 100; i++) { 
    zeroValue[0] += analogRead(aX);
    zeroValue[1] += analogRead(aY);
    delay(10);
  }  
  zeroValue[0] /= 100;
  zeroValue[1] /= 100;
}

/****************************************************/
/*                                                  */
/*           Posicionado de particulas              */
/*                                                  */
/****************************************************/
void mueve_bolas(){
  //Actualizamos posiciones de particulas 
  for (int o = 0;o < numeroBolitas; o++){
    //Colisiones con los limites del ejeX 
    bolita[o].velX += (LoX/4)*bolita[o].masa; //Reducimos a un 0.25 el valor del acelerometro
                                              //y variamos la velocidad respecto a la masa
    bolita[o].posX += bolita[o].velX;         // Calculamos nueva posicion ejeX
    //Comprobamos colisiones con los extremos de la matriz
    if (bolita[o].posX < 0) {        //Comprobamos si hay rebote en punto X=0
        bolita[o].posX = 0;          //Posicionamos en X=0
        bolita[o].velX = elasticidad*bolita[o].velX; //Invertimos la direccion del vector de 
                                                     //velocidad y le aplicamos un coeficiente
                                                     //de reduccion (elasticidad)
    }
    //rebote en el otro extremo del ejeX procedemos igual
    if (bolita[o].posX > ((anchoDisplay-1)*factorReduccion)) { 
        bolita[o].posX = (anchoDisplay-1)*factorReduccion;
        bolita[o].velX = elasticidad*bolita[o].velX;
    }
    //Colisiones con los limites del ejeY
    //Procedemos igual que con el eje X
    bolita[o].velY += (LoY/4)*bolita[o].masa;
    bolita[o].posY += bolita[o].velY;
    
    if (bolita[o].posY < 0) {        
        bolita[o].posY = 0;
        bolita[o].velY = elasticidad*bolita[o].velY;
    }

    if (bolita[o].posY > ((altoDisplay-1)*factorReduccion)) {  
        bolita[o].posY = (altoDisplay-1)*factorReduccion;
        bolita[o].velY = elasticidad*bolita[o].velY;
    }
  }
}

/****************************************************/
/*                                                  */
/*        Colisiones entre particulas               */
/*                                                  */
/****************************************************/
// Vamos a ir comprobando la distancia entre particulas. En caso de ser
// d < 1 calculamos los nuevos vectores de velocidad resultantes usando
// trigonometria y vectores.
// El calculo es: 
// particula 1 con la 2,3,4,5,6,...,n
// particula 2 con la 3,4,5,6,....,n (con la 1 ya esta calculado)
// .....
// particula n-1 con la n
void colision_bolas(){
  for (int o = 0;o < numeroBolitas-1; o++){ //particula a comparar
      for (int i = o + 1; i < numeroBolitas; i++) { //resto de particulas
      //calculamos la distancia entre particulas usando Pitagoras
      float dx = bolita[i].posX/factorReduccion - bolita[o].posX/factorReduccion;
      float dy = bolita[i].posY/factorReduccion - bolita[o].posY/factorReduccion;
      float distancia = sqrt(dx*dx + dy*dy);
      //Si colisionan, calculamos los nuevos vectores de velocidad resultantes
      if (distancia < 1) { //colision
        float angle = atan2(dy, dx);  //Angulo de la colision
        //Calculo de la nueva posicion
        float targetX = bolita[o].posX/factorReduccion + cos(angle) ;
        float targetY = bolita[o].posY/factorReduccion + sin(angle) ;
        //Desplazamiento producido y aplicacion de un coeficiente de
        //reduccion por la friccion entre particulas
        float ax = (targetX - bolita[i].posX/factorReduccion) * friccion;
        float ay = (targetY - bolita[i].posY/factorReduccion) * friccion;
        //Ajuste de las velocidades en base al desplazamiento generado
        //La que impacta reduce su velocidad y la impactada la incrementa
        //Habeis visto las bolas de billar cuando chocan???
        bolita[o].velX -= ax*bolita[o].masa;
        bolita[o].velY -= ay*bolita[o].masa;
        bolita[i].velX += ax*bolita[i].masa;
        bolita[i].velY += ay*bolita[i].masa;
      }
    } 
  }
}

/****************************************************/
/*                                                  */
/*                Dibujar particulas                */
/*                                                  */
/****************************************************/
void pon_bolas(){
  matrix.clearScreen(); //Borra matriz
  for (int o = 0;o < numeroBolitas; o++){
   matrix.drawPixel(bolita[o].posX/factorReduccion, bolita[o].posY/factorReduccion, 1);
  }
  matrix.writeScreen(); //Escribe matriz
}


/****************************************************/
/*                                                  */
/*                Funciones de Debug                */
/*                                                  */
/****************************************************/
