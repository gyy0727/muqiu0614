#include "../include/Log.h"
#include "../include/http_server.h"

static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

#define XX(...) #__VA_ARGS__

IOManager::ptr worker;
void run() {
  HttpServer::ptr server(
      new HttpServer(true, worker.get(), IOManager::getThis(),IOManager::getThis()));
  // HttpServer::ptr server(new HttpServer(true));
  Address::ptr addr = Address::LookupAnyIPAddress("0.0.0.0:8020");
  while (!server->bind(addr)) {
    sleep(2);
  }
  auto sd = server->getServletDispatch();
  sd->addServlet("/", [](HttpRequest::ptr req, HttpResponse::ptr rsp,
                         HttpSession::ptr session) {
    rsp->setBody(req->toString() + R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>My Test Page</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f4f4f9;
            color: #333;
        }

        header {
            background-color: #333;
            color: white;
            padding: 10px 20px;
            text-align: center;
        }

        h1 {
            margin: 0;
        }

        nav {
            background-color: #444;
            color: white;
            padding: 10px 20px;
            text-align: center;
        }

        nav a {
            color: white;
            text-decoration: none;
            margin: 0 10px;
        }

        nav a:hover {
            text-decoration: underline;
        }

        main {
            padding: 20px;
            max-width: 800px;
            margin: 20px auto;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            background-color: white;
        }

        section {
            margin-bottom: 20px;
        }

        footer {
            background-color: #333;
            color: white;
            text-align: center;
            padding: 10px 20px;
        }
    </style>
</head>
<body>
    <header>
        <h1>Welcome to My Test Page</h1>
    </header>
    <nav>
        <a href="#">Home</a>
        <a href="#">About</a>
        <a href="#">Contact</a>
    </nav>
    <main>
        <section>
            <h2>About Us</h2>
            <p>This is a simple test page to showcase some basic styling and layout.</p>
        </section>
        <section>
            <h2>Features</h2>
            <ul>
                <li>Responsive design</li>
                <li>Clean and modern look</li>
                <li>Easy navigation</li>
            </ul>
        </section>
        <section>
            <h2>Contact Information</h2>
            <p>Email: info@example.com</p>
            <p>Phone: +1234567890</p>
        </section>
    </main>
    <footer>
        &copy; 2024 My Test Page. All rights reserved.
    </footer>
</body>
</html>)rawliteral");
    return 0;
  });

  sd->addGlobServlet("/*", [](HttpRequest::ptr req, HttpResponse::ptr rsp,
                              HttpSession::ptr session) {
    rsp->setBody(R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>My Test Page 2</title>
    <style>
        body {
            font-family: 'Roboto', sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f0f0f0;
            color: #333;
        }

        header {
            background-color: #007BFF;
            color: white;
            padding: 15px 20px;
            text-align: center;
        }

        h1 {
            margin: 0;
        }

        nav {
            display: flex;
            justify-content: space-around;
            background-color: #0056b3;
            color: white;
            padding: 10px 0;
        }

        nav a {
            color: white;
            text-decoration: none;
            font-size: 18px;
        }

        nav a:hover {
            text-decoration: underline;
        }

        main {
            padding: 20px;
            max-width: 800px;
            margin: 20px auto;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            background-color: white;
        }

        section {
            margin-bottom: 20px;
        }

        footer {
            background-color: #007BFF;
            color: white;
            text-align: center;
            padding: 10px 20px;
        }

        .container {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-top: 20px;
        }

        .card {
            border: 1px solid #ccc;
            border-radius: 5px;
            padding: 15px;
            background-color: white;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
        }

        .card img {
            width: 100%;
            height: auto;
            border-radius: 5px;
        }
    </style>
</head>
<body>
    <header>
        <h1>Welcome to My Test Page 2</h1>
    </header>
    <nav>
        <a href="#">Home</a>
        <a href="#">About</a>
        <a href="#">Contact</a>
    </nav>
    <main>
        <section>
            <h2>About Us</h2>
            <p>This is a simple test page to showcase some basic styling and layout with a different theme.</p>
        </section>
        <section>
            <h2>Features</h2>
            <ul>
                <li>Responsive design</li>
                <li>Clean and modern look</li>
                <li>Easy navigation</li>
            </ul>
        </section>
        <section class="container">
            <div class="card">
                <img src="https://via.placeholder.com/250x150" alt="Sample Image">
                <h3>Card Title</h3>
                <p>Some quick example text to build on the card title and make up the bulk of the card's content.</p>
            </div>
            <div class="card">
                <img src="https://via.placeholder.com/250x150" alt="Sample Image">
                <h3>Card Title</h3>
                <p>Some quick example text to build on the card title and make up the bulk of the card's content.</p>
            </div>
            <div class=".card">
                <img src="https://via.placeholder.com/250x150" alt="Sample Image">
                <h3>Card Title</h3>
                <p>Some quick example text to build on the card title and make up the bulk of the card's content.</p>
            </div>
        </section>
    </main>
    <footer>
        &copy; 2024 My Test Page 2. All rights reserved.
    </footer>
</body>
</html>)rawliteral");
    return 0;
  });

  sd->addGlobServlet("/sylarx/*", [](HttpRequest::ptr req,
                                     HttpResponse::ptr rsp,
                                     HttpSession::ptr session) {
    rsp->setBody(
        XX(<html><head><title> 404 Not Found</ title></ head><body><center>
                   <h1> 404 Not Found</ h1></ center><hr><center>
                       nginx /
                   1.16.0 <
               / center > </ body></ html> < !--a padding to disable MSIE and
           Chrome friendly error page-- > < !--a padding to disable MSIE and
           Chrome friendly error page-- > < !--a padding to disable MSIE and
           Chrome friendly error page-- > < !--a padding to disable MSIE and
           Chrome friendly error page-- > < !--a padding to disable MSIE and
           Chrome friendly error page-- > < !--a padding to disable MSIE and
           Chrome friendly error page-- >));
    return 0;
  });

  server->start();
}

int main(int argc, char **argv) {
  IOManager iom(3, "main");
  worker.reset(new IOManager(16, "worker"));
  iom.schedule(run);
  return 0;
}
