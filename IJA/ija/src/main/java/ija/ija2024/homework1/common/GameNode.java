package ija.ija2024.homework1.common;

public abstract class GameNode {
    protected Position position;

    public GameNode(Position position) {
        this.position = position;
    }

    public Position getPosition() {
        return this.position;
    }

    public abstract boolean isBulb();

    public abstract boolean isLink();

    public abstract boolean isPower();

    public abstract void turn();

    public abstract boolean containsConnector(Side s);
}