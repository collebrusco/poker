# poker thing or something
shoutout eva   
This is a state machine based poker implementation with an abstracted player interface.   
The poker backend state machine is entirely separate from any user or AI controller code, or any rendering code.  
### writing players 
Players make their moves through an interface, you can easily take a crack at writing a poker-playing AI by overriding these two methods in PokerPlayerController.
```c++
virtual PokerBetAction* bet(PokerState const& game, PokerPlayer const& player) = 0;
virtual ControlResult discard(PokerState const& game, PokerPlayer const& player) = 0;
```
you can return one of these bet actions to make your move.
```c++
struct PokerBetAction {
    virtual void perform(PokerState& game) = 0;
};
struct CheckAction  : public PokerBetAction;
struct CallAction   : public PokerBetAction;
struct RaiseAction  : public PokerBetAction;
struct FoldAction   : public PokerBetAction;
struct AllInAction  : public PokerBetAction;
```
I don't have any AI's written yet, only an implementation that asks the user to make their move in the console.    
### use the backend
This is how one instantiates and runs a game, but of course you'd have varied player types in reality, whether human or AI. You can see how you could simulate large numbers of games between different AIs to compare them.
```c++
PlayerList players;

players.add(new ConsolePlayer());
players.add(new ConsolePlayer());
players.add(new ConsolePlayer());

PokerGame game(players);

game.run().print();
```

## frontend / renderer
I am building a proper renderer / frontend for this game which will have a PokerPlayerController implementation so the user can play thru a gui. TBD

## notes
this is just notes for me    
cards png was made for free nicely by someone [here](https://devforum.play.date/t/playing-card-deck-imagetable-free-for-your-card-game/994)     
### dims
the card sheet is:     
| unit | W   | H     |
|-----|-----|------|
| cards | 13  | 5  |
| pix | 650 | 350  |
| pix/card | 50  | 70 |

### layout

|  | 0    | 1      | 2    | 3    | 4    | 5    | 6    | 7    | 8    | 9    | 10   | 11  | 12  |
|----|------|--------|------|------|------|------|------|------|------|------|------|-----|-----|
| 0  | - | Jk  | b1 | b2 | b3 |  -   |  -   |  -   |  -   |  -   |  -   |  -  |  -  |
| 1  | Ah   | 2h     | 3h   | 4h   | 5h   | 6h   | 7h   | 8h   | 9h   | 10h  | Jh   | Qh  | Kh  |
| 2  | Ad   | 2d     | 3d   | 4d   | 5d   | 6d   | 7d   | 8d   | 9d   | 10d  | Jd   | Qd  | Kd  |
| 3  | As   | 2s     | 3s   | 4s   | 5s   | 6s   | 7s   | 8s   | 9s   | 10s  | Js   | Qs  | Ks  |
| 4  | Ac   | 2c     | 3c   | 4c   | 5c   | 6c   | 7c   | 8c   | 9c   | 10c  | Jc   | Qc  | Kc  |

### img
![cards](res/cards.png)
