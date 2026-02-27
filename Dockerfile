# Usa un'immagine Debian Linux iper-leggera
FROM debian:bookworm-slim

# Installa il compilatore C, SQLite e Lighttpd (il server web di produzione)
RUN apt-get update && apt-get install -y gcc libsqlite3-dev lighttpd

# Imposta la cartella di lavoro standard dei server web
WORKDIR /var/www/html

# Copia tutto il tuo progetto nella cartella web
COPY . /var/www/html/

# Assicurati che la cartella cgi-bin esista
RUN mkdir -p /var/www/html/cgi-bin

# Compila il C e rinominalo come standard web (.cgi)
RUN gcc -o /var/www/html/cgi-bin/backend.cgi cgi-bin/backend.c -lsqlite3

# Dai i permessi massimi di esecuzione al binario
RUN chmod 755 /var/www/html/cgi-bin/backend.cgi

# Crea una cartella esterna sicura per il Database e dalle i permessi massimi
RUN mkdir -p /var/www/data && chmod 777 /var/www/data

# Inietta la configurazione di Lighttpd
RUN echo 'server.document-root = "/var/www/html"' > /etc/lighttpd/lighttpd.conf && \
    echo 'server.port = 8080' >> /etc/lighttpd/lighttpd.conf && \
    echo 'server.modules += ( "mod_cgi", "mod_alias" )' >> /etc/lighttpd/lighttpd.conf && \
    echo 'cgi.assign = ( ".cgi" => "" )' >> /etc/lighttpd/lighttpd.conf && \
    echo 'alias.url = ( "/cgi-bin/" => "/var/www/html/cgi-bin/" )' >> /etc/lighttpd/lighttpd.conf

# Espone la porta (utile per documentazione Docker)
EXPOSE 8080

# Al lancio: Railway passa la porta nella variabile $PORT. 
# Sovrascriviamo la porta nel file di configurazione e avviamo il server.
CMD sed -i "s/server.port = 8080/server.port = ${PORT:-8080}/" /etc/lighttpd/lighttpd.conf && lighttpd -D -f /etc/lighttpd/lighttpd.conf