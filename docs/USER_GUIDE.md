# Guia do utilizador

## Iniciar

```bash
./build/banking_system
```

## Criar conta

No menu principal escolha **1. Create account**, selecione o tipo:

| Tipo | Taxa levantamento | Taxa transferência | Overdraft | Juro anual |
|------|-------------------|--------------------|-----------|------------|
| Checking | 0.50 | 1.00 | 200.00 | 0% |
| Savings | 1.50 | 0.00 | 0 | 2% |
| Business | 0.00 | 0.1% (mín. 2.00) | 5000.00 | 0.5% |

Anote o **Account ID** gerado (ex.: `ACC1000`).

## Login

Escolha **2. Login** e introduza o ID. Não é pedida password — isto é intencional (demo).

## Operações (após login)

- **Deposit / Withdraw / Transfer** — montantes no formato `100` ou `100.50`
- **Balance** — saldo atual e limites
- **Transaction history** — movimentos da conta
- **Update personal data** — nome, email, telefone
- **Generate statement** — escreve `data/statement_<ID>.txt`

## Bloquear conta

No menu principal (sem sessão): **4. Block / unblock account**.

Contas bloqueadas rejeitam depósitos, levantamentos e transferências.

## Dados persistentes

Ficheiros em `data/` (ou no diretório passado como argumento):

- `accounts.csv`
- `transactions.csv`
- `banking.log`
- `statement_*.txt`
