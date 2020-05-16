# Informe del Trabajo Práctico 1
### Alumno: Francisco Ruiz. Padrón:99429
### Link: https://github.com/FRuiz811/tp1.git

En el siguiente informe se presentará un resumén de como se encaró la 
resolución del problema planteado en el trabajo práctico. Se adjuntarán 
diagramas descriptivos que intentarán ayudar a la explicación del mismo.
Por último, detallaré algunos inconvenientes a los cuales me enfrente y cómo 
resolví la cuestión.

## Descripción de la Resolución
El siguiente diagrama nos presenta un pantallazo general de como está 
conformada la resolución de la comunicación del Cliente/Servidor utilizando el 
protocolo D-Bus:

![Diagrama General](img/Diagrama_General.png)

Si bien no está detallado a nivel de la implementación, si nos permite ver 
como el cliente y el servidor hacen uso del protocolo para codificar y 
decodificar el mensaje que será enviado a través de un socket que previamente 
tuvo que ser conectado de manera exitosa.

Una importante decisión de diseño, es el uso del protocolo como traductor y no 
como el encargado de realizar el envío/recepción del mensaje, esto es 
responsabilidad exclusiva del cliente o del servidor.

A continuación explicaré a grandes rasgos los pasos que sigue el proceso:

El cliente se conectará al host y al puerto indicados. Es el encargado de leer
el archivo de entrada, ya sea a través de la entrada estándar o aquel que fue 
indicado como parámetro. Lo irá leyendo de a 32 bytes y una vez que se 
garantice de que ya leyó una línea, recurrirá al protocolo para realizar la 
traducción correspondiente.

Una vez en el protocolo, se parseará el mensaje tomando como delimitador el 
espacio " ". Dependiendo de que parte del mensaje se trate, se setearán el 
tipo de parámetro y el tipo de dato para codificarlo según rigen las reglas 
del protocolo D-Bus. Una vez finalizado el encoding del mensaje, se lo 
devolverá al cliente junto con la longitud del mismo.

Ya con el mensaje encodeado, el cliente procederá a enviar a través de su 
socket el mismo, para luego quedar aguardando la confirmación del servidor.

El siguiente diagrama ejemplifica la secuncia recién explicada.

![Diagrama Cliente/Servidor](img/Diagrama_Cliente_Servidor.png)

Ahora, situemosnos en el lado del servidor. Este será ejecutado y recibirá 
como parámetro el puerto en el cual escuchará las conexiones entrantes. 
El servidor se conectará a través de un socket al puerto solicitado y alli 
aguardará los mensajes del cliente. Una vez que llegue la solicitud para 
conectarse, el socket aceptará la conexión y nos dará un nuevo socket por el 
cual vamos a poder establecer una comunicación con el cliente.

Una vez conectados, el servidor quedará a la espera de recibir 16 bytes, con 
los cuales obtendrá toda la información necesaria acerca de la longuitud del 
cuerpo, el id del mensaje y la longitud del array de parámetros. Recibidos 
estos bytes, serán enviados al protocolo que será el encargado de decodificar 
la información descripta previamente.

Ya con la información del id, de las longitudes del array y del cuerpo del 
mensaje, el servidor procederá a recibir los siguientes bytes que serán tantos 
como la longitud del array lo disponga. 

Con estos bytes, se volverá a llamar al protocolo, pero esta vez para 
decodificar los parámetros que tendrá nuestro mensaje. 

De igual manera, será el tratamiento de los bytes correspondientes al cuerpo 
del mensaje. 

Con todo esto procesado, el servidor habrá recibido el mensaje enviado y 
continuará notificandole al cliente que llegó correctamente.

El siguiente diagrama ejemplifica la secuncia recién explicada.

![Diagrama Servidor/Cliente](img/Diagrama_Servidor_Cliente.png)


## Problemas Encontrados

### Codificación del Protocolo
Debido a la diversidad de longitudes de cada uno de los partes del mensaje y 
la variabilidad de como pueden venir los parámetros, estos pueden estar 
separados por ", " o solamente por "," me encontré con el trabajo de seccionar 
correctamente cada parte sin perder la información y luego cada una de estas 
codificarla de la manera correspondiente. 

Esta parte fue la más engorrosa donde no podía vislumbrar la manera correcta 
de codificar cada una de las partes individualmente y luego unirlas. En ese 
momento, fue cuando opté por generar un buffer dinámico, ya utilizado en la 
lectura del archivo de entrada, dentro del protocolo. Esto me permitió ir 
concatenando los bytes de manera contigua pudiendo independizar las 
codificaciones de cada parte, ya sea para el array de parámetros, el padding o 
el cuerpo del mensaje.

### Recibir información en el Server
El problema en esta parte, también radicó en las diferentes longitudes que 
pueden tener los mensajes, en este caso me afectaría la manera en que los iba 
a recibir. Por eso, decidí dividir la recepción y la decodificación del 
mensaje en 3 partes:

+ Información del Mensaje: se reciben 16 bytes que luego serán enviados al 
protocolo para que traduzca esta información en ID, longitud del array, 
longitud del cuerpo.
+ Header: se reciben tantos bytes como la longitud del array lo disponga. Esto 
se envía al protoco para recibir cada uno de las partes del mensaje separada 
de las demás (destino, ruta, interfaz, método).
+ Cuerpo: En caso de que exita, se reciben tantos bytes como la longuitud del 
cuerpo nos indique. Esto se vuelve a enviar al protocolo para tener cada uno 
de los parámetros separados.

## Aclaraciones
Esta sección está integramente dedicada a realizar aclaraciones acerca de la 
entrega.

### Normas de Codificación
Al intentar ejecutar las normas de codificación, me surgió el siguiente error:

![Error Normas Codificación](img/ErrorNormas.png)

Este error me obligó a desarrollar la función _split_ para poder cumplir las 
reglas cppflint, reemplazando de esta manera la función utilizada en un 
principio que era strtok.

### Longitud de Funciones

Si bien se pedía que las funciones no superasen las 20 línea, en algunas no 
pude respetar esa regla debido a que no vi como modularizar aún más las 
mismas. Por otro lado, los controles de errores y las estructuras de control   
(switch case) de algunas funciones hacen que la extensión de estas sobrepase 
facilmente las 20 líneas. Aquí voy a explicar aquellas que sobrepase de 
sobremanera las 20 líneas:


+ protocol_encode_message: esta función es la encarga de despachar cada parte 
del mensaje a la función que corresponda para que sea encodeado de la manera 
correcta. Su tarea principal es dividir correctamente el mensaje recibido como 
parámetro y llamar a las funciones que le dan el formato al protocolo. Hace 
las veces de un "dispatcher", para luego devolver el mensaje encodeado según 
las reglas del protocolo D-Bus.

+ protocol_decode_header: En este caso, más de la mitad de las líneas se las 
lleva la estructura de control (switch case). Es una función muy simple que 
solamente decodifica el mensaje recibido a medida que lo va leyendo.

## Correcciones para Segunda Entrega

Para la segunda entrega se pidieron corregir 3 cosas:

+ Uso de constantes y defines: esto se envidenció mucho en la parte del 
protocolo, donde se definieron constantes para cada tipo de parámetro y el 
tipo de dato del mismo. En otra parte que se aplicaron estas correcciones fue 
en el servidor, donde se definió del buffer de lectura y del máximo padding 
posible.

+ Limpiar Headers inutilizados: en algunos archivos, habían quedado header que 
fueron incluídos con algún propósito a lo largo del desarrollo pero que a la 
hora de la entrega final, no tenían ningún sentido. Por otro lado, había 
librerias que se incluían tanto en el .h como en el .c, en aquellos casos en 
los que fue posible, se dejó la librería en el .c para evitar incluír 
librerias innecesarias. 

+ Refactor de la función protocol_encode_message: se generó una función 
privada llamada encode_dispatch, que es la encargada de distribuir cada parte 
del mensaje a la función que corresponda para que sea encodeado de la manera 
correcta. Ahora protocol_encode_message se encarga solamente de dividir el mensaje recibido y devolverlo encodeado junto con su longitud.  