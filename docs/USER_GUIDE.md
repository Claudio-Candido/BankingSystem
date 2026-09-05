# Guia do utilizador

## Iniciar

```bash
./build/sistema_bancario
```

## Criar conta

No menu principal escolha **1. Criar conta**, selecione o tipo:

| Tipo | Taxa levantamento | Taxa transferência | Descoberto | Juro anual |
|------|-------------------|--------------------|------------|------------|
| Corrente | 0.50 | 1.00 | 200.00 | 0% |
| Poupança | 1.50 | 0.00 | 0 | 2% |
| Empresa | 0.00 | 0.1% (mín. 2.00) | 5000.00 | 0.5% |

Anote o **ID da conta** gerado (ex.: `ACC1000`).

## Iniciar sessão

Escolha **2. Iniciar sessão** e introduza o ID. Não é pedida password — isto é intencional (demo).

## Operações (após iniciar sessão)

- **Depositar / Levantar / Transferir** — montantes no formato `100` ou `100.50`
- **Saldo** — saldo atual e limites
- **Histórico de transações** — movimentos da conta
- **Atualizar dados pessoais** — nome, email, telefone
- **Gerar extrato** — escreve `data/extrato_<ID>.txt`

## Bloquear conta

No menu principal (sem sessão): **4. Bloquear / desbloquear conta**.

Contas bloqueadas rejeitam depósitos, levantamentos e transferências.

## Dados persistentes

Ficheiros em `data/` (ou no diretório passado como argumento):

- `contas.csv`
- `transacoes.csv`
- `banca.log`
- `extrato_*.txt`
