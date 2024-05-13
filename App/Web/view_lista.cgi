t <!DOCTYPE html>
t <html lang="es">
t <head>
t  <meta charset="UTF-8">
t  <title>Registro de Usuarios</title>
# IMPORTA UN ARCHIVO LLAMADO XML_HTTP.JS
t <script language=JavaScript type="text/javascript" src="xml_http.js"></script> 
#INICIA UN BLOQUE DE SCRIPT EN JAVASCRIPT
t <script language=JavaScript type="text/javascript"> 
t var formUpdate = new periodicObj("view_lista.cgx", 500); 
t function plotADGraph() { 
t  linealeida = document.getElementById("fila_value").value;
t  var lineaseparada = linealeida.split(",");
t  document.getElementById("fila_value").value = lineaseparada[3];
t }
t function periodicUpdateAd() {
t  if(document.getElementById("adChkBox").checked == true) {
t   updateMultiple(formUpdate,plotADGraph);
t   ad_elTime = setTimeout(periodicUpdateAd, formUpdate.period);
t  }
t  else
t   clearTimeout(ad_elTime);
t }
# window.onload = cadenaSplitted();
t </script>
t  <style>
t     table, th, td {
t        border: 1px solid black;
t        border-collapse: collapse;
t     }
t     th, td {
t        padding: 8px;
t        text-align: left;
t    }
t  </style>
t </head>
t <body>
t <form action="view_lista.cgi" method="post" name="nuevaPaginaActual">
#Botón de página anterior
t  <input type="submit" name="nuevaPaginaActual" value="<<">
#Pagina actual
c h <span> Pagina actual: %d </span>
#Botón de página siguiente
t  <input type="submit" name="nuevaPaginaActual" value=">>">
#Pagina actual
c i <span> Ultima pagina: %d </span>
# Campos ocultos para mantener el estado
t <input type="hidden" value="lista" name="pg">
t    <h1>Registro de Usuarios</h1>
t    <table>
t 	 <tbody>
t    	<tr>
t          <th>Nombre</th>
t          <th>Apellido</th>
t          <th>ID</th>
t          <th>Dentro</th>
t     	</tr>
#        <!-- Aquí se insertarán las filas de la tabla -->
t   <tr><td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c z 1 	size="40" id="Nombre1" value = "%s"></td>
t   <td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c z 2 	size="40" id="Apellido1" value = "%s"></td>
t   <td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c z 3 	size="40" id="ID1" value = "%s"></td>
t   <td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c z 4 	size="40" id="Inside1" value = "%s"></td></tr>
#
t   <tr><td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c m 1 	size="40" id="Nombre2" value = "%s"></td>
t   <td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c m 2 	size="40" id="Apellido2" value = "%s"></td>
t   <td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c m 3 	size="40" id="ID2" value = "%s"></td>
t   <td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c m 4 	size="40" id="Inside2" value = "%s"></td></tr>
#
t   <tr><td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c n 1 	size="40" id="Nombre3" value = "%s"></td>
t   <td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c n 2 	size="40" id="Apellido3" value = "%s"></td>
t   <td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c n 3 	size="40" id="ID3" value = "%s"></td>
t   <td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c n 4 	size="40" id="Inside3" value = "%s"></td></tr>
#
t   <tr><td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c o 1 	size="40" id="Nombre4" value = "%s"></td>
t   <td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c o 2 	size="40" id="Apellido4" value = "%s"></td>
t   <td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c o 3 	size="40" id="ID4" value = "%s"></td>
t   <td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c o 4 	size="40" id="Inside4" value = "%s"></td></tr>
#
t   <tr><td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c q 1 	size="40" id="Nombre5" value = "%s"></td>
t   <td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c q 2 	size="40" id="Apellido5" value = "%s"></td>
t   <td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c q 3 	size="40" id="ID5" value = "%s"></td>
t   <td align="center">
t 	<input type="text" readonly style="background-color: transparent; border: 0px"
c q 4 	size="40" id="Inside5" value = "%s"></td></tr>
t 	</tbody>
t   </table>
t </p></form>
t </body>
t </html>
.