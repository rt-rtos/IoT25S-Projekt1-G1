# Why the project is set up the way it is

1. Why is the code split into so many small files and folders?
2. Why is there a board with 49 cards for a four-week project?
3. Why does the code contain functions that are empty?
4. Why were the "rules" for each part written before the part itself?


## 1. What was done in week 1

- The project was laid out in folders: one for code that only runs on the
  Arduino board, one for code that can run on any computer, one for tests,
  one for the PC-side software, one for documents, and one for the
  manufacturer's example programs we base our drivers on.
- For every part of the system, we wrote down what it promises to the
  rest of the system before writing the part itself: what goes in, what
  comes out, how long it may take, and what happens when it fails.
- Every part that does not need real hardware was written and tested on a
  PC. Nine automatic tests pass.
- Every part that does need hardware or network exists as a placeholder
  that compiles and does nothing yet, with a note inside saying exactly
  what needs to be done and which manufacturer example to copy from.
  There are 24 such notes.
- The whole program builds for the board. The PC-side software starts
  with one command.
- Each note became a card on the board, sorted by who owns it. Hardware
  work, tests, decisions and documents got cards of their own.

## 2. The folder structure

### What we did

The code is sorted two ways at once. First, by whether it needs the
board: code that touches a pin, the Wi-Fi radio or a sensor cable lives in
one folder; code that is pure logic (turning raw sensor bytes into a
temperature, checking whether a value is reasonable, building the message
that gets sent) lives in another. Second, by who owns it: each person's
work sits in its own folder.

Settings we expect to adjust, like how often to measure, live in one file.
The Wi-Fi password lives in a file that is never uploaded to the shared
repository.

### Why

The pure-logic code can be run and tested on any laptop in seconds,
without a board on the desk. That is why it was finished and tested in
week 1, before all the hardware had arrived, and why it can be re-tested
after every change at no cost. The board-only code can only be checked on
the board, so it is kept apart and kept small.

Sorting by owner means there is always one person to ask about any file,
and two people never edit the same file at the same time.

### What it is called

Keeping hardware-free logic separate so it can be tested on a PC is
standard practice in embedded development, where the hardware is often
late, scarce or shared. The folder layout itself is the convention of the
build tool we use, which is the most common one for Arduino projects
outside the basic Arduino editor. Organising code by team structure
follows an old observation in software: a system always ends up mirroring
the team that built it, so it is cheaper to plan for that than to fight
it.

## 3. The board

### What we did

A board with four columns and 49 cards. Every card says which area it
belongs to, who owns it, which file it touches, and where possible what
"finished" looks like. The cards were written from the notes in the code
and from the test list in the design document, so the board and the code
describe the same work.

### Why

Five people work in parallel for four weeks. Without a shared list, each
person knows their own work and guesses at everyone else's. The board
shows at a glance what is left, who is doing it, and what is waiting on a
decision. It also gives the standup something concrete to walk through.

The cards are small on purpose. A card that takes half a day visibly
moves across the board. A card that takes two weeks sits in one column
and tells nobody anything. Small cards can also be handed to someone else
when a person is ill or ahead of schedule.

The "Unassigned" column holds work whose owner is still an open decision.
Saying so openly is better than everyone assuming someone else has it.

### What it is called

A kanban board. The idea comes from factory management and was adopted by
software teams: make the work visible, limit how much is in progress at
once, and pull the next card when one is done. The course's project plan
asks for a backlog and a time plan; the board is the backlog in a form
that stays up to date. Writing "done when" on a card is what agile teams
call a definition of done. Being able to follow a line from the
customer's letter to a requirement to a design section to a card to a
test result is called traceability, and it is required in any industry
where software safety matters.

## 4. The empty functions

### What we did

Every part of the system exists with its final name, in its final place,
and the whole thing compiles and runs. Parts that could be finished
without hardware are finished. Parts that could not are placeholders that
compile, do nothing harmful, and carry a note listing the steps, the
relevant section of the design document, and the manufacturer example to
adapt.

Example. The Wi-Fi module has a function that is supposed to keep the
connection alive. Right now it reads:

```
void Network::poll(uint32_t nowMs) {
    // TODO(network): refresh status_ from WiFi.status(); while not
    // connected, call WiFi.begin(ssid_, pass_) at most every retryMs_
    // (use lastAttemptMs_). See examples/ConnectWithWPA and outline 5.3.
    (void)nowMs;
}
```

The main program already creates the Wi-Fi module, starts it, and
expects to call this function in every state. When Ali fills in the body
and Linus fills in the main program's state handling, neither has to
touch the other's file.

### Why

Think of building a house. You put up the whole frame first, with every
room and doorway in place, and then fill in the walls. The alternative is
five people each building a room somewhere in the yard for two weeks and
then trying to push them together. Everyone who has done that once knows
how it ends: the doors do not line up, and fitting them together eats the
time that was meant for finishing.

With the frame in place, the fitting-together already happened in week
1, when it was cheap. From now on, every part that gets finished drops
into a system that already builds, already runs, already produces
messages and already has a receiver waiting. The first real Wi-Fi
connection will show up on the dashboard the same day it works, because
everything around it is already there.

The frame also protects the design. The way the program cycles, waits and
reacts is already written into the main program. Someone filling in a
part has to fit that shape. They cannot quietly write something that
freezes the whole board for a second without noticing that nothing around
it expects that.

The notes inside the placeholders are instructions, not "figure it out
later". Each points at a manufacturer example that already does most of the job.
What is left is adapting a known example to a known shape, which is work that can be sized and put on a card.

### What it is called

A walking skeleton: a thin version of the whole system that goes end to
end and does almost nothing, built first so that everything added later
lands in a working whole. Some people call the same idea a tracer bullet.
Placeholders that compile and return a safe default are called stubs, and
they are the normal way to build against a part that is not ready yet.

## 5. Rules first, code second

### What we did

Before any part was written, we wrote down what it promises to the rest
of the system. The clearest case is the rule set every sensor driver
follows. It says, for each of the three sensors:

- Starting the sensor reports whether it answered.
- Asking for a measurement returns immediately; it does not wait.
- There is a way to ask whether the result is ready yet.
- Reading the result always stamps a time and a fault code, gives "no
  value" if the sensor did not answer, and never holds the board up for
  more than a few thousandths of a second.

Three drivers by three people follow that one rule set, and the part that
coordinates them was written against the rules alone, before any driver
existed.

The same was done at every other seam: the exact text of the message sent
to the PC, the names of the database columns, how long connecting may
block, and that a sanity check may flag a value as suspicious but never
change it.

### Why

The rules are the part of a module other people depend on. The inside is
the part only its owner depends on. Settling the rules first means the
thing five people rely on was decided while everyone was in the same
room, and the inside can be changed by its owner at any time without
asking anyone.

What happens on failure is the part that usually gets forgotten and is
the most expensive to add afterwards. Deciding on day one that an
unplugged sensor produces "no value" plus a fault code, and never a crash,
a stale number or a zero, is what makes fault handling work across the
whole chain: the driver reports it, the message carries it, the database
stores it, the dashboard shows it. Nobody has to arrange that in week 3.

Written rules are also what made the PC tests possible. A test for the
message builder is a test of its promise: this data in, exactly this text
out. Without a written promise, a test can only check what the code
happens to do today.

### What it is called

Design by contract, or contract-first development. The same idea in web
development is called API-first: agree on the exact shape of the data
between two systems before either is built, so both teams can work at the
same time. A sensor's datasheet is a contract too, and one of our cards
exists only to double-check the numbers we took from them.

## 6. Objections and answers

**"Most of the code is empty, so nothing is done."**
What is done is the part that is hardest to change later: the structure,
the rules between parts, the message format, the build, the tests, the
PC side, and every module that could be finished without hardware. What
is left is the part that is easiest to size and hand out. The week 1 goal
in the time plan was "frame builds, PC tests run, receiver up", and it is
met.

**"This is a lot of process for a four-week school project."**
The process is one board and one document. Neither adds a meeting beyond
the standup the course already requires. Filling the board took an
afternoon, and it pays that back the first time someone is unsure what to
do next. A short project with several people has less room for surprises
at the end than a long one, which makes the frame more valuable, not
less.

**"Why not just start coding and see what we need?"**
Five people coding without a shared frame produce five frames. The frame
cost a few days of one person's time. Merging five separately grown code
bases in week 3 costs an unknown amount. The customer also asks for a
design another developer can understand; a frame where the rules are
written in the code satisfies that from day one.

**"Empty functions are dead code."**
Dead code is code nothing uses. These are the opposite: the main program
already creates and starts every one of them, the build is clean, and
each carries the instructions for its own completion.

**"The rules will turn out to be wrong."**
Some will, and that is expected. The design document already changed
between its second and third version for this reason. The point of
writing rules down is that when one changes, the change is visible,
discussed, and applied everywhere at once, instead of being discovered
inside someone's code when the parts are put together.

**"A real company would not do this for a prototype."**
A real company with five engineers, hardware that has not arrived and a
customer demo in four weeks would do exactly this. The walking skeleton
was invented for small teams on short projects, and PC-side testing of
embedded code was invented for teams whose hardware is late. Both
describe us.
