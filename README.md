# Flip7

Modeling the Flip7 Card Game.

## Description

This is a comprehensive enviroment for testing strategies for the Card game [Flip7](https://theop.games/pages/flip-7?srsltid=AU7gw4VmVf2CjvMCrBdrUqu0KNoJEMBWQeXk4h4TSvUR8n3yPJYKrgLq)

## Overview

This project will contain three parts. First, the gametheoretic modeling made by hand, which gives a concrete structure of the game which then is implemented in part two via Cpp. The goal of the implementation is to give a framework in which strategies can be tested via Monte Carlo Simulations. Cpp was chosen, since the computations are quite intense, especially for the expected value functions and a initial python implementation couldn't handle this intensity.
Part three deepdives into some proposed strategies, tested via the Monte Carlo simulations and gives recommended strategies for players playing without computers and/or complete information.


## Modeling
The Modeling is done via a Markov Decision Process (MDP) which has a proven optimum via the Bellman Optimality Theorem. The goal of this part is to construct a mathematical framework of the game in which optimal decisions can be made. This guides the implementation process and gives some overview over time and space complexity.


## Implementation
In this part I implemented the game with its mechanisms to run the games. It is separated into the infrastructure, i.e. the game mechanics and the player part. The latter of which provides the opportunity to run strategies. One should note, that via the first part, in particular the Bellman Equation, optimal decisions, based on the game state are proven if the player keeps track of the game. 
Since this is not always the case in human games, my goal is to provide human calculable strategies, which sufficiently approximate the optimal choice.


## Authors

Contributors names and contact info 
ex. [@JesseGvhp](https://github.com/JesseGvhp)

## Version History

* 0.1
    * Initial Release

