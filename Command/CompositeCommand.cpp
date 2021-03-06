#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

struct BankAccount{
    int balance{0};
    int overdraft_limit{-500};
    void deposit(int amount)
    {
        balance +=amount;
        cout<<"deposited"<<amount<<", balance is now"<<balance<<endl;
    }
    bool withdraw(int amount){
        if(balance - amount >= overdraft_limit)
        {
            balance -= amount;
            cout<<"withdrew"<<amount<<", balance is now"<<balance<<endl;
            return true;
        }
        return false;
    }
    friend ostream &operator <<(ostream &os, const BankAccount &account){
        os<< "balance:"<<account.balance;
        return os;
    }
};

struct Command
{
    bool succeeded;
    virtual void call() = 0;
    virtual void undo() = 0;
};

struct BankAccountCommand : Command{
    BankAccount& account;
    enum Action{deposit, withdraw} action;
    int amount;
    BankAccountCommand(BankAccount &account, Action action, int amount) : amount(amount), action(action),account(account){
        succeeded = false;
    }
    void call() override{
        switch (action){
            case deposit:
                account.deposit(amount);
                succeeded = true;
                break;
            case withdraw:
                succeeded = account.withdraw(amount);
                break;
        }
    }
    void undo() override{
        if(!succeeded) return;
        switch (action) {
            case deposit:
                account.withdraw(amount);
                break;
            case withdraw:
                account.deposit(amount);
                break;
        }
    }
};
struct CompositeBankAccountCommand : vector<BankAccountCommand>, Command
{
  CompositeBankAccountCommand(const initializer_list<BankAccountCommand> &items) :
  vector(items){}
  void call() override{
      for(auto& cmd : *this){
          cmd.call();
      }
  }
  void undo() override{
      for(auto it = rbegin();it != rend(); ++it)
      {
          it -> undo();
      }
  }
};

struct DependentCompositeCommand : CompositeBankAccountCommand
{
    DependentCompositeCommand(const initializer_list<BankAccountCommand> &items) :
    CompositeBankAccountCommand(items){}
    void call() override{
        bool ok = true;
        for (auto& cmd : *this){
            if(ok){
                cmd.call();
                ok =cmd.succeeded;
            }
            else{
                cmd.succeeded = false;
            }
        }
    }
};

struct MoneyTransferCommand : DependentCompositeCommand
{
    MoneyTransferCommand(
        BankAccount& from,
        BankAccount& to,
        int amount
    ) : DependentCompositeCommand(
        {
            BankAccountCommand{from, BankAccountCommand::withdraw, amount},
            BankAccountCommand{to, BankAccountCommand::deposit, amount},
        }
    ){}
};

int main(){
    BankAccount ba1, ba2;
    ba1.deposit(100);

    MoneyTransferCommand cmd{ba1, ba2, 100};
    cmd.call();
    cout<<ba1<<endl<<ba2<<endl;
    system("pause");
    return 0;
}
