# Estruturação #

o trabalho está divido em camadas:

* **main** : faz inicializações e chama o application layer

* **application Layer** : cenas com os pacotes de dados

* **link Layer** : llopen, llwrite, llread, llclose (fazem a ligação com o application Layer)

# O que está feito  #

* llopen, llwrite, llread e llclose(faltam uns testes)

* byte stuffing

* stop-n-wait

# TODO list #

**Link Layer** (responsável -> Inês)

* descobrir o erro do bbc2

* colocar o stuffing para tudo

* ler o ppt para ver se não falta nenhum pormenor

* testar mais exaustivamente (ver mesmo o conteudo das tramas de um lado para o outro

* testar as cenas do n e r (embora isto esteja mais relacionada com o applicationLayer, convém ter a certeza que funciona)

**application Layer**

* reciever

* fazer testes de erros

* verificar tamanho máximo dos pacotes e do campo dos dados

* verificar se os tamanhos das string têm +1 para o \0


# Duvidas : #