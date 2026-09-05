# Banking System

Aplicação de terminal em **C++17** que simula operações bancárias. Projeto intermediário com arquitetura em camadas, pensado para demonstrar engenharia de software — **não** é um sistema bancário real.

> **Aviso de segurança:** não há autenticação real. O “login” apenas seleciona uma conta e abre uma sessão de demonstração em memória. **Nenhuma palavra-passe ou credencial é armazenada.** Não use este código em produção nem com dados reais.

## Funcionalidades

| Funcionalidade | Descrição |
|---|---|
| Criação de contas | Checking, Savings e Business |
| Login (demo) | Seleção de conta por ID — sem credenciais |
| Depósito / Levantamento | Com taxas polimórficas por tipo de conta |
| Transferência | Entre contas, com taxa no remetente |
| Consulta de saldo | Saldo, overdraft e taxa de juro |
| Histórico | Lista de transações da conta |
| Bloqueio de conta | Bloquear / desbloquear |
| Alteração de dados | Nome, email e telefone |
| Extrato | Ficheiro de texto gerado em `data/` |
| Persistência | CSV via file streams (`accounts.csv`, `transactions.csv`) |

## Arquitetura em camadas

```
┌─────────────────────────────────────┐
│  UI          ConsoleUI (terminal)   │
├─────────────────────────────────────┤
│  Business    BankingService, Session│
├─────────────────────────────────────┤
│  Domain      Account*, Transaction  │
├─────────────────────────────────────┤
│  Data        Repositories + CSV     │
└─────────────────────────────────────┘
```

Detalhes em [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Requisitos C++ demonstrados

- `class` / herança / polimorfismo (`Account` → Checking / Savings / Business)
- `std::vector`, `std::unordered_map`
- Smart pointers (`unique_ptr`, `shared_ptr`)
- Exceções tipadas (`BankingException` e derivadas)
- File streams (`fstream`) para persistência e logging
- `enum class` (`AccountType`, `TransactionType`, …)
- Templates (`IRepository<T>`, justificado como contrato CRUD reutilizável)

## Build

Requisitos: **CMake ≥ 3.16**, **g++/clang++ com C++17**.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Binário: `build/banking_system`

## Execução

```bash
./build/banking_system          # usa ./data
./build/banking_system /tmp/bs  # diretório de dados customizado
```

Fluxo típico:

1. Criar conta (guardar o ID, ex. `ACC1000`)
2. Login com o ID
3. Depositar, levantar, transferir, consultar histórico
4. Gerar extrato → `data/statement_ACC1000.txt`

## Testes

Suite em `tests/test_main.cpp` (asserts leves, sem framework externo):

- parsing de valores monetários
- CSV escape/split
- taxas polimórficas
- validação
- round-trip de persistência
- fluxo completo do serviço (depósito, levantamento, transferência, bloqueio, extrato, sessão)

```bash
./build/banking_tests
```

## Estrutura do projeto

```
BankingSystem/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── include/banking/
│   ├── common/     Types, Exceptions, Logger, Validation
│   ├── domain/     Account hierarchy, Transaction, Factory
│   ├── data/       Repositories, CSV utils
│   ├── business/   BankingService, Session, IdGenerator
│   └── ui/         ConsoleUI
├── src/main.cpp
├── tests/
├── docs/
└── data/           Criado em runtime (CSV + logs + extratos)
```

## Logging

Logs em stderr e, se possível, em `data/banking.log`.

## Licença

MIT — ver [LICENSE](LICENSE).
