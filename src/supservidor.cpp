#include <iostream>     /* cerr */
#include <algorithm>
#include "supservidor.h"

using namespace std;

/* ========================================
   CLASSE SUPSERVIDOR
   ======================================== */

/// Construtor
SupServidor::SupServidor()
  : Tanks()
  , server_on(false)
  , LU()
  , servthread()
  , servsock()
  /*ACRESCENTAR*/
{
  // Inicializa a biblioteca de sockets
  /*ACRESCENTAR*/
  // Em caso de erro, mensagem e encerra
  if (mysocket::init() != mysocket_status::SOCK_OK)
  {
    cerr <<  "Biblioteca mysocket nao pode ser inicializada";
    exit(-1);
  }
}

/// Destrutor
SupServidor::~SupServidor()
{
  // Deve parar a thread do servidor
  server_on = false;

  // Fecha todos os sockets dos clientes
  for (auto& U : LU) U.close();
  // Fecha o socket de conexoes
  /*ACRESCENTAR*/
  servsock.close();

  // Espera o fim da thread do servidor
  /*ACRESCENTAR*/
  if (servthread.joinable())
    servthread.join();

  // Encerra a biblioteca de sockets
  /*ACRESCENTAR*/
  mysocket::end();
}

/// Liga o servidor
bool SupServidor::setServerOn()
{
  // Se jah estah ligado, nao faz nada
  if (server_on) return true;

  // Liga os tanques
  setTanksOn();

  // Indica que o servidor estah ligado a partir de agora
  server_on = true;

  try
  {
    // Coloca o socket de conexoes em escuta
    /*ACRESCENTAR*/
    // Em caso de erro, gera excecao
    if (servsock.listen(SUP_PORT) != mysocket_status::SOCK_OK) throw 1;

    // Lanca a thread do servidor que comunica com os clientes
    /*ACRESCENTAR*/
    servthread = std::thread([this]() { this->thr_server_main();});

    // Em caso de erro, gera excecao
    if (!servthread.joinable()) throw 2;
  }
  catch(int i)
  {
    cerr << "Erro " << i << " ao iniciar o servidor\n";

    // Deve parar a thread do servidor
    server_on = false;

    // Fecha o socket do servidor
    /*ACRESCENTAR*/
    servsock.close();

    return false;
  }

  // Tudo OK
  return true;
}

/// Desliga o servidor
void SupServidor::setServerOff()
{
  // Se jah estah desligado, nao faz nada
  if (!server_on) return;

  // Deve parar a thread do servidor
  server_on = false;

  // Fecha todos os sockets dos clientes
  for (auto& U : LU) U.close();
  // Fecha o socket de conexoes
  /*ACRESCENTAR*/
  servsock.close();

  // Espera pelo fim da thread do servidor
  /*ACRESCENTAR*/
  if (servthread.joinable())
    servthread.join();

  // Faz o identificador da thread apontar para thread vazia
  /*ACRESCENTAR*/
  servthread = std::thread();

  // Desliga os tanques
  setTanksOff();
}

/// Leitura do estado dos tanques
void SupServidor::readStateFromSensors(SupState& S) const
{
  // Estados das valvulas: OPEN, CLOSED
  S.V1 = v1isOpen();
  S.V2 = v2isOpen();
  // Niveis dos tanques: 0 a 65535
  S.H1 = hTank1();
  S.H2 = hTank2();
  // Entrada da bomba: 0 a 65535
  S.PumpInput = pumpInput();
  // Vazao da bomba: 0 a 65535
  S.PumpFlow = pumpFlow();
  // Estah transbordando (true) ou nao (false)
  S.ovfl = isOverflowing();
}

/// Leitura e impressao em console do estado da planta
void SupServidor::readPrintState() const
{
  if (tanksOn())
  {
    SupState S;
    readStateFromSensors(S);
    S.print();
  }
  else
  {
    cout << "Tanques estao desligados!\n";
  }
}

/// Impressao em console dos usuarios do servidor
void SupServidor::printUsers() const
{
  for (const auto& U : LU)
  {
    cout << U.login << '\t'
         << "Admin=" << (U.isAdmin ? "SIM" : "NAO") << '\t'
         << "Conect=" << (U.isConnected() ? "SIM" : "NAO") << '\n';
  }
}

/// Adicionar um novo usuario
bool SupServidor::addUser(const string& Login, const string& Senha,
                             bool Admin)
{
  // Testa os dados do novo usuario
  if (Login.size()<6 || Login.size()>12) return false;
  if (Senha.size()<6 || Senha.size()>12) return false;

  // Testa se jah existe usuario com mesmo login
  auto itr = find(LU.begin(), LU.end(), Login);
  if (itr != LU.end()) return false;

  // Insere
  LU.push_back( User(Login,Senha,Admin) );

  // Insercao OK
  return true;
}

/// Remover um usuario
bool SupServidor::removeUser(const string& Login)
{
  // Testa se existe usuario com esse login
  auto itr = find(LU.begin(), LU.end(), Login);
  if (itr == LU.end()) return false;

  // Remove
  LU.erase(itr);

  // Remocao OK
  return true;
}

/// A thread que implementa o servidor.
/// Comunicacao com os clientes atraves dos sockets.
void SupServidor::thr_server_main(void)
{
  // Fila de sockets para aguardar chegada de dados
  /*ACRESCENTAR*/
  mysocket_queue fila;

  while (server_on)
  {
    // Erros mais graves que encerram o servidor
    // Parametro do throw e do catch eh uma const char* = "texto"
    try
    {
      // Encerra se o socket de conexoes estiver fechado
      if (servsock.closed())
      {
        throw "socket de conexoes fechado";
      }

      // Inclui na fila de sockets todos os sockets que eu
      // quero monitorar para ver se houve chegada de dados

      // Limpa a fila de sockets
      /*ACRESCENTAR*/
      fila.clear();

      // Inclui na fila o socket de conexoes
      /*ACRESCENTAR*/
      fila.include(servsock);

      // Inclui na fila todos os sockets dos clientes conectados
      /*ACRESCENTAR*/
      for (auto& U : LU)
      {
          if (U.isConnected())
            fila.include(U.usersock);
      }

      // Espera ateh que chegue dado em algum socket (com timeout)
      /*ACRESCENTAR*/
      auto status = fila.wait_read(SUP_TIMEOUT * 1000);


      if(status == mysocket_status::SOCK_ERROR)
        server_on = false;
      else if (status == mysocket_status::SOCK_OK)
        {
          for (auto& U : LU)
            if (U.isConnected() && fila.had_activity(U.usersock))
            {
              uint16_t cmd;

              if (U.usersock.read_uint16(cmd) != mysocket_status::SOCK_OK)
              {
                U.close();
                std::cout << "Cliente " << U.login << " desconectado." << std::endl;
              }
              else
              {
                  switch(cmd)
                  {
                    case CMD_GET_DATA:
                    {
                      SupState S;
                      readStateFromSensors(S);

                      if (U.usersock.write_uint16(CMD_DATA) != mysocket_status::SOCK_OK)
                        U.close();

                      else
                        if (U.usersock.write_bytes((mybyte*)&S, sizeof(S)) != mysocket_status::SOCK_OK)
                          U.close();

                      break;
                    }

                    case CMD_SET_V1:
                    case CMD_SET_V2:
                    case CMD_SET_PUMP:
                    {
                      uint16_t param;

                      if (U.usersock.read_uint16(param) != mysocket_status::SOCK_OK)
                        U.close();

                      else
                      {

                        if (U.isAdmin)
                        {

                          if (cmd == CMD_SET_V1)   std::cout << "CMD_SET_V1 " << param << " DE " << U.login << " (OK)" << std::endl;
                          else if (cmd == CMD_SET_V2) std::cout << "CMD_SET_V2 " << param << " DE " << U.login << " (OK)" << std::endl;
                          else if (cmd == CMD_SET_PUMP) std::cout << "CMD_SET_PUMP " << param << " DE " << U.login << " (OK)" << std::endl;


                          if (cmd == CMD_SET_V1) setV1Open(param);
                          else if (cmd == CMD_SET_V2) setV2Open(param);
                          else setPumpInput(param);


                          if (U.usersock.write_uint16(CMD_OK) != mysocket_status::SOCK_OK)
                            U.close();
                        }
                        else
                        {
                          if (cmd == CMD_SET_V1)   std::cout << "CMD_SET_V1 " << param << " DE " << U.login << " (ERRO)" << std::endl;
                          else if (cmd == CMD_SET_V2) std::cout << "CMD_SET_V2 " << param << " DE " << U.login << " (ERRO)" << std::endl;
                          else if (cmd == CMD_SET_PUMP) std::cout << "CMD_SET_PUMP " << param << " DE " << U.login << " (ERRO)" << std::endl;

                          if (U.usersock.write_uint16(CMD_ERROR) != mysocket_status::SOCK_OK)
                            U.close();
                        }
                      }
                      break;
                    }

                    case CMD_LOGOUT:
                    {
                        std::cout << "Cliente " << U.login << " fez logout." << std::endl;
                        U.close();
                        break;
                    }

                    default:
                    {
                      if(U.usersock.write_uint16(CMD_ERROR) != mysocket_status::SOCK_OK)
                        U.close();
                      break;
                    }

                  }
              }

            }
          if (fila.had_activity(servsock))
          {
            tcp_mysocket temp;

            if (servsock.accept(temp) == mysocket_status::SOCK_OK)
            {
              uint16_t cmd;
              std::string login, senha;

              if (temp.read_uint16(cmd) == mysocket_status::SOCK_OK &&
                  cmd == CMD_LOGIN &&
                  temp.read_string(login) == mysocket_status::SOCK_OK &&
                  temp.read_string(senha) == mysocket_status::SOCK_OK)
              {
                  auto user_itr = std::find(LU.begin(), LU.end(), login);

                  if (user_itr != LU.end() && user_itr->password == senha &&
                      !user_itr->isConnected())
                  {
                    user_itr->usersock.swap(temp);

                    uint16_t resposta = (user_itr->isAdmin ? CMD_ADMIN_OK : CMD_OK);

                    if (user_itr->usersock.write_uint16(resposta) != mysocket_status::SOCK_OK)
                      user_itr->close();

                    else
                    {
                      std::cout << "Cliente " << user_itr->login << " conectou com sucesso."
                      << std::endl;
                    }
                  }

                  else
                  {
                    std::cout << "Tentativa de login falhou para o usuario " << login << "." << std::endl;
                    temp.write_uint16(CMD_ERROR);
                    temp.close();
                  }
              }


            }
          }
        }


      // De acordo com o resultado da espera:
      // SOCK_TIMEOUT:
      // Saiu por timeout: nao houve atividade em nenhum socket
      // Aproveita para salvar dados ou entao nao faz nada
      // SOCK_ERROR:
      // Erro no select: encerra o servidor
      // SOCK_OK:
      // Houve atividade em algum socket da fila:
      //   Testa se houve atividade nos sockets dos clientes. Se sim:
      //   - Leh o comando
      //   - Executa a acao
      //   = Envia resposta
      //   Depois, testa se houve atividade no socket de conexao. Se sim:
      //   - Estabelece nova conexao em socket temporario
      //   - Leh comando, login e senha
      //   - Testa usuario
      //   - Se deu tudo certo, faz o socket temporario ser o novo socket
      //     do cliente e envia confirmacao

    } // fim try - Erros mais graves que encerram o servidor
    catch(const char* err)  // Erros mais graves que encerram o servidor
    {
      cerr << "Erro no servidor: " << err << endl;

      // Sai do while e encerra a thread
      server_on = false;

      // Fecha todos os sockets dos clientes
      for (auto& U : LU) U.close();
      // Fecha o socket de conexoes
      /*ACRESCENTAR*/
      servsock.close();

      // Os tanques continuam funcionando

    } // fim catch - Erros mais graves que encerram o servidor
  } // fim while (server_on)
}



