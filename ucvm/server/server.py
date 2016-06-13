##
#    Handles the web server which serves up the web interface for UCVM
#    which provides an easier way for scientists to access the software.

from ucvm.server import template
from http.server import BaseHTTPRequestHandler,HTTPServer
import webbrowser
import os

PORT = 8080

# This class will handle any incoming request from
# a browser 
class ucvmHandler(BaseHTTPRequestHandler):

    # Handler for the GET requests
    def do_GET(self):
        path = self.path.strip("/")
        filepath = os.path.dirname(os.path.abspath(__file__)) + "/html/"
        
        f = open(filepath + "theme.tpl.html", "r")
        retstr = f.read()
        
        if os.path.isfile(filepath + path + ".tpl.html"):
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            
            f = open(filepath + path + ".tpl.html", "r")
            retstr = retstr.replace("[content]", f.read())
            self.wfile.write(bytes(template.perform_replacements(retstr), "utf-8"))
        elif path == "":
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            
            f = open(filepath + "home.tpl.html", "r")
            retstr = retstr.replace("[content]", f.read())
            self.wfile.write(bytes(template.perform_replacements(retstr), "utf-8"))            
        else:
            self.send_response(404)
            self.send_header('Content-type','text/html')
            self.end_headers()
            
            self.wfile.write(bytes("<html><body>404!</body></html>", "utf-8"))
            
        return
    
    # Handler for the POST requests.
    def do_POST(self):
        return

def start_server():
    try:
        # Create a web server and define the handler to manage the
        # incoming request
        server = HTTPServer(('', PORT), ucvmHandler)
        print ('Started httpserver on port ' , PORT)

        webbrowser.open("http://localhost:8080")

        # Wait forever for incoming http requests
        server.serve_forever()

    except KeyboardInterrupt:
        print ('^C received, shutting down the web server')
        server.socket.close()