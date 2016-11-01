Se combinarem ir para a feup testar nos pcs do lab avisem-me pff.
Obg, Filipe

# Relatório #

https://docs.google.com/document/d/1kmi_Fj4mGASmqxhOTDoC3Hnxkl3D-4IZNzam-zsMRIo/edit

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

* seq number do link layer do receiver devia começar em 1 senão a primeira trama I recebida dá um warning de retransmissão

* retransmissões detectadas que tenham sido recebidas previamente com sucesso devem ser ignoradas (enviar apenas resposta)

* receiver fica indefinidamente a enviar a trama DISC - descobrir erro


* descobrir o erro do bbc2

* colocar o stuffing para tudo

* ler o ppt para ver se não falta nenhum pormenor

* testar mais exaustivamente (ver mesmo o conteudo das tramas de um lado para o outro

* testar as cenas do n e r (embora isto esteja mais relacionada com o applicationLayer, convém ter a certeza que funciona)

**application Layer**

* a ordem da info do start package (tamanho, nome, data, permissoes) deve ser indiferente segundo o professor (o receiver deve ser capaz de ler a start package independentemente da ordem)


# Duvidas : #