# Usa un'immagine Linux leggera con Python preinstallato (per il server web)
FROM python:3.11-slim

# Installa il compilatore C e le librerie di sviluppo di SQLite
RUN apt-get update && apt-get install -y gcc libsqlite3-dev

# Imposta la cartella di lavoro sul server
WORKDIR /app

# Copia tutti i file del tuo progetto (html, css, cartella cgi-bin) nel server
COPY . /app/

# Compila il file C e trasformalo in un eseguibile
RUN gcc -o cgi-bin/backend cgi-bin/backend.c -lsqlite3

# Dai i permessi di esecuzione al file appena compilato
RUN chmod +x cgi-bin/backend

# Railway assegna una porta dinamica tramite la variabile $PORT.
# Avviamo il server abilitando i CGI su quella specifica porta.
CMD sh -c "python3 -m http.server --cgi ${PORT:-8000}"