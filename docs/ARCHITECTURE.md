# Architecture

## Layers

### UI (`banking/ui`)

- `ConsoleUI`: menus de terminal, leitura de input, apresentação de resultados.
- Não contém regras de negócio; delega tudo a `BankingService`.
- Traduz exceções de domínio em mensagens amigáveis.

### Business Logic (`banking/business`)

- `BankingService`: orquestra criação de contas, movimentos, bloqueio, extratos e sessão.
- `Session` / `SessionManager`: sessão **em memória**. Abrir sessão = selecionar conta por ID. Sem verificação de password.
- `IdGenerator`: IDs de conta (`ACC…`), transação (`TX…`) e token de sessão demo.

### Domain (`banking/domain`)

- `Account` (abstrata): saldo, estado, dados do titular; interface polimórfica para taxas, juro e overdraft.
- `CheckingAccount`, `SavingsAccount`, `BusinessAccount`: regras distintas.
- `AccountFactory`: cria a subclasse correta a partir de `AccountType`.
- `Transaction`: registo imutável de movimento.

### Data (`banking/data`)

- `IRepository<T>`: contrato template de repositório.
- `AccountRepository`: `unordered_map` em memória + persistência CSV.
- `TransactionRepository`: índice por conta + lista global; CSV.
- `CsvUtil`: escape/split CSV.

### Common (`banking/common`)

- `enum class` para tipos e estados.
- Exceções tipadas (`ValidationException`, `InsufficientFundsException`, …).
- `Logger` (singleton thread-safe).
- Validação de nome, email, telefone e montantes (centavos inteiros).

## Money

Valores monetários usam `MoneyCents` (`int64_t`) para evitar erros de vírgula flutuante. A UI aceita strings como `100.50`.

## Persistence format

`accounts.csv`:

```
id,type,holderName,email,phone,balanceCents,status
```

`transactions.csv`:

```
id,accountId,type,amountCents,balanceAfterCents,description,relatedAccountId,timestamp
```

## Error handling

Operações de negócio validam entrada e estado da conta. Falhas lançam exceções derivadas de `BankingException`, capturadas na UI e registadas no logger.

## Demo session policy

Este projeto **não** implementa autenticação. Em produção, a identidade seria validada por um serviço externo (OAuth, IdP, etc.). Aqui, o login existe apenas para estruturar o menu pós-seleção de conta.
