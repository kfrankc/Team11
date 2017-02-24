import sys, os, subprocess
import requests
import time

class ExpectedResponse: 
  body = ""
  content_type = ""
  status = ""

  def __init__(self, body, content_type, status):
    self.body = body
    self.content_type = content_type
    self.status = status

def send_request(req, expected):
  print 'Performing the following request: ', req
  try:
    r = session.get(req)
  except requests.exceptions.ConnectionError:
    time.sleep(1)
    r = session.get(req)
  except requests.exceptions.RequestException as e:
    serv.kill()
    print e
    print 'Connection refused'
    sys.exit(1)

  print 'Status code is:',r.status_code
  print 'Content-type is:',r.headers['content-type']

  # Checker headers
  if r.status_code != expected.status:
    print 'ERROR: Request to website failed'
    sys.exit(1) 

  # Check content-type
  if r.headers['content-type'] != expected.content_type:
    print 'ERROR: Incorrect content type'
    sys.exit(1) 

  # Check response
  if expected.body in r.text:
    print 'Expected response received!'
  else:  
    print 'ERROR: Incorrect response: '
    print r.text
    sys.exit(1)


DEVNULL = open(os.devnull, 'wb')

print 'Running integration test on webserver...'

print 'Building the binary...'
if subprocess.call(["sudo","make"]) != 0:
  print 'ERROR: Build failed'
  sys.exit(1)

print 'Running webserver...'
serv = subprocess.Popen(["./serve", "new_config"])

print 'Sending requests to server...'
# Proxy issue fix:
session = requests.Session()
session.trust_env = False

echo = ExpectedResponse("GET /echo HTTP/1.1\r\nHost: localhost:9999", "text/plain", 200)
echo_test = ExpectedResponse("GET /echo/test HTTP/1.1\r\nHost: localhost:9999", "text/plain", 200)
static = ExpectedResponse("<html>", "text/html", 200)
bad = ExpectedResponse("<html><body><h1>404 Not Found</h1></body></html>", "text/html", 404)


send_request('http://localhost:9999/echo', echo)
# Test longest prefix mapping: 
send_request('http://localhost:9999/echo/test', echo_test)
send_request('http://localhost:9999/static1/hello.html', static)
send_request('http://localhost:9999/static1/missing', bad)

print 'Terminating webserver...'
serv.kill(); 
