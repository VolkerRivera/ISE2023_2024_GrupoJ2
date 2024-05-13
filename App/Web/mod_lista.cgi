t <html><head><title>Gestion de acceso</title>
t <meta name="viewport" content="width=device-width, initial-scale=1">
t <script language=JavaScript>
t </script></head>
i pg_header.inc
t <h2 align=center><br>Gestion de acceso</h2>
t <p><font size="2">Esta pagina sirve para anadir o eliminar usuarios a la lista
t  de acceso permitido.<br><br>
#
t  Este formulario permite <b>INCLUIR</b> mas personas a la lista de acceso.</font></p>
t <body>
t    <h1>Formulario de Registro</h1>
#	action es la url a la que va tras pulsar enviar
#FORMULARIO PARA AÑADIR GENTE
t   <form action="mod_lista.cgi" method="POST">
t        <label for="nombre">Nombre:</label>
t        <input type="text" maxlength = 20 id="nombre" name="nombre" required><br><br>
#
t        <label for="apellidos">Primer apellido:</label>
t        <input type="text" maxlength = 20 id="apellidos" name="apellidos" required><br><br>
# 		el dni del for y de id son el mismo
t        <label for="dni">Id:</label>
t        <input type="text" maxlength = 3 id="dni" name="dni" required><br><br>
#
t        <label for="password">Codigo PIN:</label>
t        <input type="password" maxlength = 4  id="password" name="password" required><br><br>
#
t        <label for="rfid">TAG:</label>
t        <select id="rfid" name="rfid" required>
t            <option value="B3:43:ED:AA:B7">Tarjeta 1</option>
t            <option value="63:22:EE:A6:09">Tag 1</option>
t            <option value="F3:7B:83:A5:AE">Tag 2</option>
t            <option value="D3:80:BF:A6:A4">Tag 3</option>
t            <option value="F3:D9:7B:A5:F4">Tag 4</option>
t            <option value="23:2B:C7:A6:69">Tag 5</option>
t            <option value="E3:CE:4E:A6:C5">Tag 6</option>
t            </select>
t            <br><br>
#
t 		 <label for="dentro">Se encuentra dentro del edificio?</label><br>
t      	 <input type="radio" id="dentro_si" name="dentro" value="1" required>Si<br>
t        <input type="radio" id="dentro_no" name="dentro" value="0" required>No<br><br>
#
t        <input type="submit" value="Incluir">
t   </form>
t <p><font size="2">
t  Este formulario permite <b>ELIMINAR</b> personas de la lista de acceso.</font></p>
#FORMULARIO PARA ELIMINAR GENTE
t   <form action="mod_lista.cgi" method="POST">
# lo que se envia es el name -> ej "dni_delete=12345678A"
t        <label for="dni_delete">ID:</label>
t        <input type="text" maxlength = 3 id="dni_delete" name="dni_delete" required><br><br>
#
t        <label for="codigoPIN_delete">Codigo PIN:</label>
t        <input type="password" maxlength = 4  id="codigoPIN_delete" name="codigoPIN_delete" required><br><br>
#
t        <input type="submit" value="Eliminar">
t    </form>
t </body>
i pg_footer.inc
. End of script must be closed with period.